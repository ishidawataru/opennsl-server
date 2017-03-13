#ifndef demo_COORD_HPP_INCLUDED
#define demo_COORD_HPP_INCLUDED 1

/**
   The coord class is a basic x/y coordinate point class.
*/
class coord
{
private:
	int m_x;
	int m_y;

public:
	coord( int _x, int _y ) : m_x(_x),m_y(_y)
	{
	}
	coord() : m_x(0),m_y(0)
	{
	}


	int x() const { return this->m_x; }
	int y() const { return this->m_y; }

	void x( int _x ) { this->m_x = _x; }
	void y( int _y ) { this->m_y = _y; }

	bool operator==( const coord & rhs ) const
	{
		if( &rhs == this ) return true;
		return (rhs.m_x == this->m_x) && (rhs.m_y == this->m_y );
	}

	/**
	   Defined so we can use coord objects as keys in sorted
	   containers (e.g., maps).

	   Returns true only if this object's x and y are both
	   smaller than their rhs counterparts.
	*/
	bool operator<( const coord & rhs ) const
	{
		if( &rhs == this ) return false;
		return (this->m_x < rhs.m_x) && (this->m_y < rhs.m_y );
	}
};


#endif // demo_COORD_HPP_INCLUDED
