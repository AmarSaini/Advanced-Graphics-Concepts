# pragma once

// Viewer for this example:
class Sphere
{  
	public :
		GsVec center;
		float radius;
		float radiusSquared;
		GsVec surfaceColor;
		GsVec emissionColor;
		float transparency;
		float reflection;

		Sphere() {

		}

		Sphere(GsVec c, float r, GsVec surfaceC, GsVec emissionC, float reflect, float transp) {
			center = c;
			radius = r;
			radiusSquared = r * r;
			surfaceColor = surfaceC;
			emissionColor = emissionC;
			reflection = reflect;
			transparency = transp;
		}

		bool intersect(GsVec rayOrigin, GsVec rayDirection, float &t0, float &t1) {
			
			GsVec l = center - rayOrigin;
			
			float tca = l.x * rayDirection.x + l.y * rayDirection.y + l.z * rayDirection.z;

			if (tca < 0) {
				return false;
			}
			
			float d2 = (l.x * l.x + l.y * l.y + l.z * l.z) - (tca * tca);

			if (d2 > radiusSquared) {
				return false;
			}

			float thc = (float)sqrt(radiusSquared - d2);

			t0 = tca - thc;
			t1 = tca + thc;

			if (t0 < 0) {
				t0 = t1;
			}

			return true;

		}
};

