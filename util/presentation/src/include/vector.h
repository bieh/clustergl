/*******************************************************************************
							BSOD2 Client - vector.h
							
 Defines some simple 2D/3D math objects that can perform various 3D operations
*******************************************************************************/

/*********************************************
			3D point class
**********************************************/
class Vector3{
public:
	float x; 
	float y;
	float z;
	
	Vector3(float X,float Y,float Z){
		x=X; y=Y; z=Z;
	}
	Vector3(){
		x=y=z=0;
	}
	
	Vector3 operator+(Vector3 vVector){
        return Vector3(vVector.x + x, vVector.y + y, vVector.z + z);
    }

    Vector3 operator-(Vector3 vVector){
        return Vector3(x - vVector.x, y - vVector.y, z - vVector.z);
    }
    
    Vector3 operator*(float num){
        return Vector3(x * num, y * num, z * num);
    }

    Vector3 operator/(float num){
        return Vector3(x / num, y / num, z / num);
    }
    
    Vector3 clone(){
    	return Vector3(x,y,z);
    }
    
    float distTo(Vector3 p){    
    	float xd = p.x - x;
		float yd = p.y - y;
		float zd = p.z - z;
    
    	return sqrt(xd*xd + yd*yd + zd*zd);
    }
    
     float fastDistTo(Vector3 p){    
    	float xd = p.x - x;
		float yd = p.y - y;
		float zd = p.z - z;
    
    	return xd*xd + yd*yd + zd*zd;
    }
    
    Vector3 cross(Vector3 vVector2){
		Vector3 vNormal;

		vNormal.x = y*vVector2.z - z*vVector2.y;
		vNormal.y = z*vVector2.x - x*vVector2.z;
		vNormal.z = x*vVector2.y - y*vVector2.x;

		return vNormal;
	}
	
	float mag(){
		return (float)sqrt(x*x + y*y + z*z);
	}
	
	Vector3 normalized(){
		return Vector3(clone() / mag());
	}
		
	float dot(Vector3 p){
		return x*p.x + y*p.y + z*p.z;
	}
    
    std::string str(){
    	//return "(" + toString(x) + "," + toString(y) + "," + toString(z) + ")";
    	return "";
    }
    
    float innerProduct(Vector3 p){
		return x * p.x + y * p.y + z * p.z;	
	}
    
};


/*********************************************
			2D point class
**********************************************/
class Vector2{
public:
	float x; 
	float y;
	float z;
	
	Vector2(float X,float Y){
		x=X; y=Y;
	}
	Vector2(){
		x=y=0;
	}
	
	Vector2 operator+(Vector2 vVector){
        return Vector2(vVector.x + x, vVector.y + y);
    }

    Vector2 operator-(Vector2 vVector){
        return Vector2(x - vVector.x, y - vVector.y);
    }
    
    Vector2 operator*(float num){
        return Vector2(x * num, y * num);
    }

    Vector2 operator/(float num){
        return Vector2(x / num, y / num);
    }
    
    Vector2 clone(){
    	return Vector2(x,y);
    }
    
    float distTo(Vector2 p){    
    	float xd = p.x - x;
		float yd = p.y - y;
    
    	return sqrt(xd*xd + yd*yd);
    }
    
     float fastDistTo(Vector2 p){    
    	float xd = p.x - x;
		float yd = p.y - y;

    	return xd*xd + yd*yd;
    }
       
	
	float mag(){
		return (float)sqrt(x*x + y*y);
	}
	
	Vector2 normalized(){
		return Vector2(clone() / mag());
	}
		
	float dot(Vector2 p){
		return x*p.x + y*p.y;
	}
    
    std::string str(){
    	//return "(" + toString(x) + "," + toString(y) + "," + toString(z) + ")";
    	return "";
    }
    
};

/*********************************************
			Quaternion class
**********************************************/
class Quaternion{
public:
	float w, x, y, z;
	
	Quaternion(float W, float X, float Y, float Z){
		w=W; x=X; y=Y; z=Z;
	}
	
	Quaternion(){
		x=y=z=0;
		w=1; //identity
	}
};

