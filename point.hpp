#ifndef POINT_HPP
#define POINT_HPP

class point {
      public:
      point(int x, int y): x(x),y(y){}
      point(){}
      bool operator==( point const& p ) const { return x==p.x && y==p.y; }
      bool operator!=( point const& p ) const{ return !( x==p.x && y==p.y ); }
      friend std::ostream& operator<<(std::ostream& out, point const& p){
        out<<"("<<p.x <<","<<p.y<<")";
        return out;
      }
      int x = 0;
      int y = 0;
  };

//stringify


#endif