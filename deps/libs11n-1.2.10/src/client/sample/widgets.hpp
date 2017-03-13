#ifndef my_WIDGETS_HPP_INCLUDED
#define my_WIDGETS_HPP_INCLUDED 1

namespace my {

	class Widget
	{
	public:
		typedef std::list<Widget *> WidgetList;
		typedef std::pair<int,int> Point;

		Widget( Widget * parent ) : m_parent(parent) {}
		Widget() : m_parent(0) {}
		virtual ~Widget();

		WidgetList const & children() const { return this->m_ch; }

		Point pos() const { return this->m_pos; }
		void pos( Point const & p ) { this->m_pos = p; }

		Point size() const { return this->m_sz; }
		void size( Point const & s ) { this->m_sz = s; }

		Widget * parent() { return this->m_parent; }
		void parent( Widget * p );

		void add_child( Widget * p );
		void remove_child( Widget * p );

	protected:
		WidgetList & children() { return this->m_ch; }
		void nuke_children();
	private:
		Widget * m_parent;
		WidgetList m_ch; // children
		Point m_pos;
		Point m_sz;
	};

} // namespace my


#endif // my_WIDGETS_HPP_INCLUDED
