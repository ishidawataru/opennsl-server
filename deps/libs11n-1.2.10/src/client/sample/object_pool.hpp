#ifndef pool_OBJECT_POOL_H_INCLUDED
#define pool_OBJECT_POOL_H_INCLUDED

#include <string>
#include <map>

#include <s11n.net/s11n/classload.hpp> // phoenix class
#include <s11n.net/s11n/phoenix.hpp> // phoenix class

// #define OBJECT_POOL_DEBUGGING 1

#ifndef OBJECT_POOL_DEBUGGING
#  define OBJECT_POOL_DEBUGGING 0
#endif

#define POOLDEBUG if(OBJECT_POOL_DEBUGGING) CERR << "@ " << std::hex << this << std::dec << ": "

/**
   Namespace pool encapsulates rudimentary object pooling
   functionality.

   It is 100% experimental.

   Author: stephan@s11n.net

   License: Do As You Damned Well Please

*/
namespace pool {

        /** Marker type for use with phoenix::phoenix. */
        struct sharing_context {};

        /**
           A basic object pool object. It is responsible for creating
           objects and later destroying those objects. It's intended
           usage is in an object server which might have multiple
           clients sharing objects via server-like interface, thus
           it's odd usage.

           Parameterized on:

           T - the type of objects to be pool. This type may not throw
           from it's ctors or dtors. If it does, all bets are off and
           you're on your own!

	   delete(T) must be safe. TODO: add a functor to this class to
	   handle destruction. This will allow safe pooling of, e.g.,
	   std::contianer<T*>.

        */
        template <typename T, typename KeyT = size_t>
        class object_pool
        {
        public:
                /**
                   The type of object stored in this pool.
                */
                typedef T value_type;
                /**
                   Convenience typedef representing this type.
                 */
                typedef object_pool<T,KeyT> this_type;

                /**
                   Lookup key type for objects.
                */
                typedef KeyT key_type;

                /**
                   The type used to map keys to objects.
                 */
                typedef std::map<key_type,value_type *> object_map;

//                 typedef typename object_map::iterator iterator;
                typedef typename object_map::const_iterator const_iterator;
        
                object_pool() : m_autodel( true )
                {
                        POOLDEBUG << "object_pool()\n";
                }

                /**
                   Returns the next available key for use as this class'
                   object keys, in the range (1..max(key_type)).

                   It may loop after some time (a LONG time, no pun intended),
                   but if you have software with that many objects in memory
                   you will want much more suitable code than this.
                */
                static key_type next_key()
                {
                        static key_type bob;
                        ++bob;
                        return (0 == bob) ? ++bob : bob;
                        // ^^^^ Zero is special here. Note that this
                        // won't happen until we overflow key_type.
                }

                /**
                   The map of keys to object pointers. It is provided mainly so
                   this list can be serialized by a third party (assuming it
                   contains Serializable objects).
                 */
                const object_map & map() const
                {
                        return this->m_hashish;
                }

                /**
                   Maps of keys to object pointers.
                */
                object_map & map()
                {
                        return this->m_hashish;
                }

//                 iterator begin() { return this->map().begin(); }
                const_iterator begin() const { return this->map().begin(); }

//                 iterator end() { return this->map().end(); }
                const_iterator end() const { return this->map().end(); }


                /**
                   Deletes all objects in this pool if auto_delete() == true.

                 */
                virtual ~object_pool()
                {
                        POOLDEBUG << "~object_pool()\n";
                        if( this->auto_delete() )
                        {
                                this->destroy_all();
                        }
                }

                /**
                   Sets the auto-deletion policy for the dtor.
                */
                void auto_delete( bool b ) { this->m_autodel = b; }

                /**
                   Returns the auto-deletion policy for the dtor.
                 */
                bool auto_delete() const { return this->m_autodel; }

                /**
                   Destroys all objects in this pool.
                   
                 */
                void destroy_all()
                {
                        typename object_map::iterator it = m_hashish.begin();
                        typename object_map::iterator eit = m_hashish.end();
                        for( ; eit != it; ++it )
                        {
//                    BUG: when using this->destroy_object() it can
//                    cause a hang somewhere in the STL interators,
//                    for unknown reasons!
//                                 this->destroy_object( (*it).first );
                                POOLDEBUG << "Destroying object keyed '"<<(*it).first<<"': " << std::hex << (*it).second<<"\n";
                                delete( (*it).second );
                        }
                        this->m_hashish.clear();
                        this->m_reverse.clear();
                }

                /**
                   For internal use. It is in the public API for
                   deserialization reasons.
                */
                void rebuild_backrefs()
                {
                        typename object_map::iterator it = m_hashish.begin();
                        typename object_map::iterator eit = m_hashish.end();
                        this->m_reverse.clear();
                        for( ; eit != it; ++it )
                        {
                                this->m_reverse[(*it).second] = (*it).first;
                        }
                }

                /**
                   Returns a shared instance of this type. It is
                   post-main()-safe in the sense that you will get a
                   valid object, but if it is destroyed once, any
                   phoenixed version will be empty, as it has no way
                   of phoenixing the pool's contents.
                */
                static this_type & instance()
                {
                        typedef ::s11n::Detail::phoenix<this_type,sharing_context> PHT;
                        return PHT::instance();
                }

                /**
                   Tries to create an object of the type named by
                   classname. It uses libs11n_class_loader, so see those
                   docs for what classes are supported. This class'
                   value_type is the base-most type used for
                   classloading.

                   This object owns the created object - it is part of
                   this pool. It's pool ID can be fetched via
                   get_key(), passing it the return value of this
                   function.

                   0 is returned if classname cannot be classloaded.
                */
                value_type *
                create_object( const std::string & classname, key_type key = 0 )
                {
                        //POOLDEBUG << "create_object("<<classname<<")"<<std::endl;
                        value_type * obj = ::s11n::cl::classload<value_type>( classname );
                        if( ! obj ) return 0;
                        this->add_object( obj, key );
                        return obj;
                }

                /**
                   Adds obj to this pool, and this pool takes
                   ownership of it.

                   A key of key_type() (i.e., zero for numeric types)
                   means to generate a unique key.

                   If key is already in this pool then the original
                   object will leak, so only manually set this when
                   you absolutely know what you're doing.

                   Returns the key of the object. It is not currently
                   possible to re-key an object, and adding an object
                   more than once will always return the same key.
                */
                key_type 
                add_object( value_type * obj, key_type key = key_type() )
                {
                        if( key_type() == key )
                        {
                                key = this_type::next_key();
                        }
                        else
                        {
                                typename reverse_object_map::iterator rit = this->m_reverse.find( obj );
                                if( this->m_reverse.end() != rit )
                                { // Adding an object twice is fatal.
                                        // This check is here to accomodate types
                                        // which call add_object() from their ctors.
                                        return (*rit).second;
                                }
                        }

                        POOLDEBUG << "add_object("<<std::hex<<obj<<") key="<<key<<"\n";
                        m_hashish[key] = obj;
                        m_reverse[obj] = key;
                        return key;
                }

                /**
                   Returns the key for the given object, or zero if
                   this pool does not contain the given object.
                */
                key_type
                get_key( const value_type * object )
                {
                        typename reverse_object_map::iterator it;
                        it = m_reverse.find( const_cast<value_type *>( object ) );
                        return (it == m_reverse.end()) ? 0 : (*it).second;
                }

                /**
                   Gets the object of the given key, or 0 if this
                   pool doesn't contain an object with that key.

                   Keys are assigned via calls to add_object().
                 */
                value_type *
                find_object( const key_type key )
                {
                        typename object_map::iterator it = m_hashish.find( key );
                        return ( m_hashish.end() == it ) ? 0 : (*it).second;
                }

                /**
                   Calls relinquish_object( key ), then deletes the object.
                   Returns true if it contained the object, otherwise false.
                */
                bool destroy_object( const key_type key )
                {
                        POOLDEBUG << "destroy_object("<<key<<")"<<std::endl;
                        value_type * o = this->relinquish_object( key );
                        if( ! o ) return false;
                        delete( o );
                        return true;
                }

                /**
                   Returns destroy_object( get_key( obj ) ).
                */
                bool destroy_object( value_type * object )
                {
                        return destroy_object( this->get_key( object ) );
                }


                /**
                   Relinquishes control of this object, removing it
                   from the object pool.  Returns 0 if it did not
                   have the given child, otherwise a pointer to that
                   object, which the caller takes ownership of.

                   This would be called release_object(), but that is
                   easily misunderstood as freeing up resources, something
                   this method doesn't do.
                */
                value_type * relinquish_object( const key_type key )
                {
                        POOLDEBUG << "relinquish_object("<<key<<")"<<std::endl;
                        if( 0 == key ) return 0;
                        typename object_map::iterator it = m_hashish.find( key );
                        if( m_hashish.end() == it ) return 0;
                        value_type * v = (*it).second;
                        m_hashish.erase( it );
                        if( v )
                        {
                                typename reverse_object_map::iterator rit = m_reverse.find( v );
                                if( m_reverse.end() != rit ) m_reverse.erase( rit );
                        }
                        POOLDEBUG << "relinquished " << std::hex << v<<"\n";
                        return v;
                }

                /**
                   Like relinquish_object(key_type) but takes an object
                   pointer. Returns true if this pool contained the object,
                   else false.
                 */
                bool relinquish_object( const value_type * object )
                {
                        key_type key = this->get_key( object );
                        if( 0 == key ) return false;
                        POOLDEBUG << "relinquish_object("<<std::hex<<object<<") key="<<key<<"\n";
                        return 0 != this->relinquish_object( key );
                }

        private:
                /**
                   A reverse map of objects-to-keys, used to speed up
                   certain operations.
                */
                typedef std::map<value_type *,key_type> reverse_object_map;
                bool m_autodel;
                object_map m_hashish;
                reverse_object_map m_reverse;

        }; // class object_pool


        /**
           EXPERIMENTAL!

           A base type for types which need to register with an
           object_pool upon creation and deregister upon destruction.

           Parameterized on:

           T: must be a struct or class type and it must extend this
           type.

           Usage sample:
<code>
struct MyType : public pooled_type<MyType> {...};
</code>

	This type is not required in order to use object_pool, but
	exists for use in types which might internally create more of
	their own type - when this happens those internally-created
	objects should be registered with their object pool, and
	subclassing this type takes care of that detail.
        */
        template <typename T>
        class pooled_type
        {
        public:
                typedef T pooled_t;
                typedef object_pool<pooled_t> pool_type;
                typedef typename pool_type::key_type pool_key_type;

                /**
                   Returns pool_type::instance().
                */
                static pool_type &
                pool()
                {
                        return pool_type::instance();
                }

                /**
                   Registers this object with pool().
                 */
                pooled_type() : m_poolid(0)
                {
                        this->m_poolid = pool().add_object( static_cast<pooled_t*>(this) );
                        POOLDEBUG << "pooled_type() @ " << std::hex << this << " pool-id="<< this->m_poolid<<"\n";

                }

                /**
                   Deregisters this object from pool().
                 */
                virtual ~pooled_type()
                {
                        POOLDEBUG << "~pooled_type() @ " << std::hex << this << "\n";
                        pool().relinquish_object( static_cast<pooled_t*>(this) );
                }

                /**
                   Returns this object's unique ID in pool().

                   Note that this value is valid only as long as this
                   object remains in it's original object pool. If it
                   is moved to another pool or
                   pool().relinquish_object() is called with this
                   object then the return value is invalid.
                */
                pool_key_type pool_id() const
                {
                        return this->m_poolid;
                }

                /**
                   Do not go blithely setting this!
                */
                void pool_id( pool_key_type val ) const
                {
                        this->m_poolid = val;
                }

        private:
                pool_key_type m_poolid;

        }; // class pooled_type

} // namespace pool

#undef POOLDEBUG
#undef OBJECT_POOL_DEBUGGING
#endif // pool_OBJECT_POOL_H_INCLUDED
