
# include "my_viewer.h"
# include "teapotdata.h"
# include <vector>
# include <iostream>

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigogl/ws_run.h>

using namespace std;

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	_nbut=0;
	_animating=false;
	showTeapotNormals=true;
	showCage = true;
	showShadow = true;
	shadowUpdate = false;
	resolution = 10;
	xLight = 0.0;
	yLight = 0.5;
	zLight = -0.5;


	shadow = new SnGroup();
	shadowTransform = new SnTransform();

	// Correct index values of teapotPatches

	for (int i = 0; i < kTeapotNumPatches; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			teapotPatches[i][j] -= 1;
		}
	}

	add_ui ();
	build_scene ();
}

void MyViewer::add_ui ()
{
	UiPanel *p, *sp;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel ( "", UiPanel::HorizLeft );
	p->add ( new UiButton ( "View", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		p->add ( _nbut=new UiCheckButton ( "Normals", EvNormals ) ); 
	}
	p->add ( new UiButton ( "Animate", EvAnimate ) );
	p->add ( new UiButton ( "Exit", EvExit ) ); p->top()->separate();
}

void MyViewer::add_model ( SnShape* s, GsVec p )
{
	SnManipulator* manip = new SnManipulator;
	GsMat m;
	m.translation ( p );
	manip->initial_mat ( m );

	SnGroup* g = new SnGroup;
	SnLines* l = new SnLines;
	l->color(GsColor::orange);
	g->add(s);
	g->add(l);
	manip->child(g);

	rootg()->add(manip);
}

GsVec getNormal(vector<GsVec> controlPts, float u, float v) {
	
	GsVec dU;
	GsVec dV;

	float b0u = (1 - u) * (1 - u) * (1 - u);
	float b1u = 3 * u * (1 - u) * (1 - u);
	float b2u = 3 * u * u * (1 - u);
	float b3u = u * u * u;

	float b0du = -3 * (1 - u) * (1 - u);
	float b1du = (3 * (1 - u) * (1 - u)) - (6 * u * (1 - u));
	float b2du = (6 * u * (1 - u)) - (3 * u * u);
	float b3du = 3 * u * u;

	float b0v = (1 - v) * (1 - v) * (1 - v);
	float b1v = 3 * v * (1 - v) * (1 - v);
	float b2v = 3 * v * v * (1 - v);
	float b3v = v * v * v;

	float b0dv = -3 * (1 - v) * (1 - v);
	float b1dv = (3 * (1 - v) * (1 - v)) - (6 * v * (1 - v));
	float b2dv = (6 * v * (1 - v)) - (3 * v * v);
	float b3dv = 3 * v * v;

	vector<GsVec> uCurve;
	vector<GsVec> vCurve;

	// Find dU

	for (int i = 0; i < 4; i++)
	{
		vCurve.push_back(controlPts[i] * b0v + controlPts[i + 4] * b1v + controlPts[i + 8] * b2v + controlPts[i + 12] * b3v);
	}

	dU = b0du*vCurve[0] + b1du*vCurve[1] + b2du*vCurve[2] + b3du*vCurve[3];

	// Find dV

	for (int i = 0; i < 16; i += 4)
	{
		uCurve.push_back(controlPts[i] * b0u + controlPts[i + 1] * b1u + controlPts[i + 2] * b2u + controlPts[i + 3] * b3u);
	}

	dV = b0dv*uCurve[0] + b1dv*uCurve[1] + b2dv*uCurve[2] + b3dv*uCurve[3];

	GsVec myNormal = cross(dU, dV);

	myNormal.normalize();

	return myNormal;

}

GsVec createPatch(vector<GsVec> controlPts, float u, float v) {

	float b0u = (1 - u) * (1 - u) * (1 - u);
	float b1u = 3 * u * (1 - u) * (1 - u);
	float b2u = 3 * u * u * (1 - u);
	float b3u = u * u * u;

	float b0v = (1 - v) * (1 - v) * (1 - v);
	float b1v = 3 * v * (1 - v) * (1 - v);
	float b2v = 3 * v * v * (1 - v);
	float b3v = v * v * v;

	vector<GsVec> uCurve;
	GsVec vPt;

	// Calc the uCurve along the 16 control points
	for (int i = 0; i < 16; i+=4)
	{
		uCurve.push_back(controlPts[i]*b0u + controlPts[i+1]*b1u + controlPts[i+2]*b2u + controlPts[i+3]*b3u);
	}

	// Calc the u,v point (v point along the uCurve that was calculated above).
	vPt = uCurve[0]*b0v + uCurve[1]*b1v + uCurve[2]*b2v + uCurve[3]*b3v;

	return vPt;

}

void MyViewer::buildTeapot(int resolution) {

	// Tested one patch first

	vector<GsVec> ControlPts(16);
	double x, y, z;
	vector< vector<GsVec> > P(resolution, vector<GsVec>(resolution));
	vector< vector<GsVec> > N(resolution, vector<GsVec>(resolution));

	for (int patchNum = 0; patchNum < kTeapotNumPatches; patchNum++) {

		for (int i = 0; i < 16; i++) {
			x = teapotVertices[teapotPatches[patchNum][i]][0];
			y = teapotVertices[teapotPatches[patchNum][i]][1];
			z = teapotVertices[teapotPatches[patchNum][i]][2];
			ControlPts[i] = GsVec(x, y, z);
			//cout << "ControlPt " << i << ": " << ControlPts[i].x << " " << ControlPts[i].y << " " << ControlPts[i].z << endl;
		}


		// i = v, j = u;
		for (int i = 0; i < resolution; i++) {
			for (int j = 0; j < resolution; j++) {
				P[i][j] = createPatch(ControlPts, float(j) / (resolution-1), float(i) / (resolution-1));
				N[i][j] = getNormal(ControlPts, float(j) / (resolution-1), float(i) / (resolution-1));
				//cout << "N[" << i << "][" << j << "]: " << N[i][j].x << " " << N[i][j].y << " " << N[i][j].z << endl;
			}
		}

		SnModel* _model = new SnModel;
		GsModel* m = _model->model();
		m->V.size(resolution*resolution);
		m->N.size(resolution*resolution);

		// Move points in P[][] to m->V[] to use m->F
		for (int i = 0; i < resolution; i++) {
			for (int j = 0; j < resolution; j++) {
				m->V[i*resolution + j].set(P[i][j]);
				m->N[i*resolution + j].set(N[i][j]);
			}
		}

		// Create Normal Lines

		SnLines* normalLines = new SnLines;
		normalLines->P.size(resolution*resolution*2);
		normalLines->color(GsColor::green);
		float normalLength = 0.1f;

		for (int i = 0; i < resolution*resolution*2; i+=2) {
			normalLines->P[i].set(m->V[i/2]);
			normalLines->P[i+1].set(m->V[i/2] + m->N[i/2]*normalLength);
		}

		// Create Cage

		SnLines* cage = new SnLines;
		cage->P.size(48);
		cage->color(GsColor::darkgray);

		// Horzontal Cage Lines
		for (int i = 0, j = 0, k = 0; i < 16; i++, j++, k+=2) {

			if (j == 3) {
				j = -1;
				k -= 2;
				continue;
			}

			cage->P[k].set(ControlPts[i]);
			cage->P[k+1].set(ControlPts[i+1]);

		}

		for (int i = 0, k = 24; i < 12; i++, k+=2) {

			cage->P[k].set(ControlPts[i]);
			cage->P[k+1].set(ControlPts[i + 4]);

		}

		// Vertical Cage Lines


		// # of triangles = (res-1)*(res-1)*2.
		// Basically, # of squares per row * # of squares per column * 2 (because 2 triangles in each square)
		// int numTriangles = (resolution - 1) * (resolution - 1) * 2;

		for (int i = 0, j = 0; i < resolution*resolution - resolution; i++, j++) {

			if (j == resolution - 1) {
				j = -1;
				continue;
			}

			m->F.push().set(i, i + 1, resolution + i);
			m->F.push().set(i + 1, resolution + i + 1, resolution + i);

		}

		m->set_mode(m->Smooth, m->NoMtl);

		GsMaterial mtl;
		mtl.diffuse = GsColor::red;
		m->set_one_material(mtl);


		SnModel* shadowModel = new SnModel;
		shadowModel->model()->add_model(*m);

		GsMaterial mtl2;
		mtl2.diffuse = GsColor::black;
		shadowModel->model()->set_one_material(mtl2);

		rootg()->add(_model);
		shadow->add(shadowModel);

		if (showTeapotNormals) {
			rootg()->add(normalLines);
		}

		if (showCage) {
			rootg()->add(cage);
		}

	}

}

GsMat MyViewer::shadowMatrix() {
	float ground[4] = { 0,0.0, -0.5,0 };
	float light[4] = { xLight, yLight, zLight,0 };

	float dot = ground[0] * light[0] + ground[1] * light[1] + ground[2] * light[2] + ground[3] * light[3];

	GsMat shadowMat(dot - ground[0] * light[0], 0 - ground[1] * light[0], 0 - ground[2] * light[0], 0 - ground[3] * light[0],
		0 - ground[0] * light[1], dot - ground[1] * light[1], 0 - ground[2] * light[1], 0 - ground[3] * light[1],
		0 - ground[0] * light[2], 0 - ground[1] * light[2], dot - ground[2] * light[2], 0 - ground[3] * light[2],
		0 - ground[0] * light[3], 0 - ground[1] * light[3], 0 - ground[2] * light[3], dot - ground[3] * light[3]);

	return shadowMat;
}

void MyViewer::build_scene() {

	if (shadowUpdate) {
		GsMat shadowMat = shadowMatrix();
		shadowTransform->get().set(shadowMat);
		shadowUpdate = false;
	}

	else {

		double startTime;
		double finishTime;

		startTime = gs_time();

		rootg()->remove_all();

		shadow = new SnGroup();
		shadowTransform = new SnTransform();

		shadow->add(shadowTransform);

		// Creates and adds the model, normals, and cage to rootg(). Adds base shadow model to shadow.
		buildTeapot(resolution);

		GsMat shadowMat = shadowMatrix();

		shadowTransform->get().set(shadowMat);

		if (showShadow) {
			rootg()->add(shadow);
		}

		finishTime = gs_time();

		cout << "Resolution: " << resolution << endl;
		cout << "Elapsed Time: " << finishTime - startTime << endl;

	}

}

// Below is an example of how to control the main loop of an animation:
void MyViewer::run_animation ()
{
	if ( _animating ) return; // avoid recursive calls
	_animating = true;
	
	int ind = gs_random ( 0, rootg()->size()-1 ); // pick one child
	SnManipulator* manip = rootg()->get<SnManipulator>(ind); // access one of the manipulators
	GsMat m = manip->mat();

	double frdt = 1.0/30.0; // delta time to reach given number of frames per second
	double v = 4; // target velocity is 1 unit per second
	double t=0, lt=0, t0=gs_time();
	do // run for a while:
	{	while ( t-lt<frdt ) t = gs_time()-t0; // wait until it is time for next frame
		double yinc = (t-lt)*v;
		if ( t>2 ) yinc=-yinc; // after 2 secs: go down
		lt = t;
		m.e24 += (float)yinc;
		if ( m.e24<0 ) m.e24=0; // make sure it does not go below 0
		manip->initial_mat ( m );
		render(); // notify it needs redraw
		ws_check(); // redraw now
	}	while ( m.e24>0 );
	_animating = false;
}

void MyViewer::show_normals ( bool b )
{
	// Note that primitives are only converted to meshes in GsModel
	// at the first draw call.
	GsArray<GsVec> fn;
	SnGroup* r = (SnGroup*)root();
	for ( int k=0; k<r->size(); k++ )
	{	SnManipulator* manip = r->get<SnManipulator>(k);
		SnShape* s = manip->child<SnGroup>()->get<SnShape>(0);
		SnLines* l = manip->child<SnGroup>()->get<SnLines>(1);
		if ( !b ) { l->visible(false); continue; }
		l->visible ( true );
		if ( !l->empty() ) continue; // build only once
		l->init();
		if ( s->instance_name()==SnPrimitive::class_name )
		{	GsModel& m = *((SnModel*)s)->model();
			m.get_normals_per_face ( fn );
			const GsVec* n = fn.pt();
			float f = 0.33f;
			for ( int i=0; i<m.F.size(); i++ )
			{	const GsVec& a=m.V[m.F[i].a]; l->push ( a, a+(*n++)*f );
				const GsVec& b=m.V[m.F[i].b]; l->push ( b, b+(*n++)*f );
				const GsVec& c=m.V[m.F[i].c]; l->push ( c, c+(*n++)*f );
			}
		}  
	}
}

int MyViewer::handle_keyboard ( const GsEvent &e )
{
	int ret = WsViewer::handle_keyboard ( e ); // 1st let system check events
	if ( ret ) return ret;

	switch ( e.key )
	{	case GsEvent::KeyEsc : gs_exit(); return 1;
	case 'n': { showTeapotNormals = !showTeapotNormals; build_scene(); render(); return 1; }
	case 'c': { showCage = !showCage; build_scene(); render(); return 1; }
	case 's': { showShadow = !showShadow; build_scene(); render(); return 1; }
	case 'e': { xLight -= 0.1f; shadowUpdate = true; build_scene(); render(); return 1; }
	case 'r': { xLight += 0.1f; shadowUpdate = true; build_scene(); render(); return 1; }
	case 'o': { if(resolution>2) resolution--; build_scene(); render(); return 1; }
	case 'p': { resolution++; build_scene(); render(); return 1; }
		default: gsout<<"Key pressed: "<<e.key<<gsnl;
	}

	return 0;
}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{	case EvNormals: show_normals(_nbut->value()); return 1;
		case EvAnimate: run_animation(); return 1;
		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
