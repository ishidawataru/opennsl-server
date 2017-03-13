#ifndef shellish_ALIASER_H_INCLUDED
#define shellish_ALIASER_H_INCLUDED 1

#include <map>
#include <string>

namespace shellish
{
        /**
           aliaser is a helper class for mapping single-token aliases
           to arbitrary strings, e.g., similarly to typical Unix shell
           alias support. It is intended to be used in the creation
           of shell-like application interfaces, but may have uses
           elsewhere (e.g., aliases class names for class_loader<T>).
        */
        class aliaser
        {
        public:
                /**
                   The map used to store aliases.
                 */
                typedef std::map<std::string,std::string> map_type;

                aliaser();
                ~aliaser();

                /**
                   Returns the map of aliases-to-expansions.
                */
                map_type & map();
                /** Const form of map(). */
                const map_type & map() const;

                /**
                   A shared instance of this object. Not post-main() safe.
                */
                static aliaser & instance();

//                 /**
//                    context() is used for getting a "context singleton" - an instance
//                    of this object which is a quasi-singleton within the given ContextType.
//                 */
//                 template <typename ContextType>
//                 static aliaser & context()
//                 {
//                         return phoenix::phoenix<aliaser,ContextType>::instance();
//                 }

                /**
                   Aliases <code>al</code> as a shortcut for <code>expanded</code>.

                   al should be exactly one token.

                   If expanded is empty then any existing alias al is removed, and
                   this is the way to unset an alias.
                */
                void alias( const std::string & al, const std::string & expanded );

                /**
                   Works similarly to Unix shells' alias expansion:

                   Expands the first input token as an alias. This
                   expansion continues until the data can no longer be
                   expanded to a new string.

                   It returns the expanded string, which may be identical
                   to the input.
                */
                std::string expand( const std::string & input ) const;

        private:
                map_type m_map;
        };

} // namespace shellish

#endif // shellish_ALIASER_H_INCLUDED
