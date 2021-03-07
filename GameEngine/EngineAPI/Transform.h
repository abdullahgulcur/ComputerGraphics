#ifndef TRANSFORM_H
#define TRANSFORM_H

class Transform {
public:

	struct position {
		float x, y, z;
	};
	struct rotation {
		float x, y, z;
	};
	struct scale {
		float x, y, z;
	};

	position position;
	rotation rotation;
	scale scale;

	Transform();
};

#endif
