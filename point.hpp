#ifndef POINT_HPP
#define POINT_HPP

class point {
      public:
      point(int x, int y): x(x),y(y){}
      point(){}
      bool operator==( point const& p ) const { return x==p.x && y==p.y; }
      bool operator!=( point const& p ) const{ return !( x==p.x && y==p.y ); }
      int x = 0;
      int y = 0;
  };

#endif