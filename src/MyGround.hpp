#ifndef MyGround_hpp
#define MyGround_hpp
#include "Object.hpp"

class MyGround : public Object {
public:
	MyGround();
protected:
	void init_meshes();
	void init_materials() {};
};

#endif