
# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigogl/ws_run.h>
# include <vector>
# include <math.h>

# include <iostream>

#define PI 3.141592654f

using namespace std;



MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	_nbut=0;
	_animating=false;
	add_ui ();
	//run_evaluation_test();
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

void MyViewer::make_custom_gear(GsModel* m, float r1, float d, int nfaces) {

	m->init();

	float r2 = r1 * 0.5f;
	float r3 = r1 + 0.2f;

	m->V.size(nfaces * 8);

	// cos/sin uses radians. Use PI/180 for conversion

	vector<float> angles;

	for (int i = 0; i < nfaces; i++) {

		angles.push_back(i * 2 * PI / nfaces);

	}

	// Vertices

	for (int i = 0; i < nfaces; i++) {

		m->V[i] = GsPnt(r2*cos(angles[i]), r2*sin(angles[i]), 0.0f);
		m->V[i + nfaces] = GsPnt(r1*cos(angles[i]), r1*sin(angles[i]), 0.0f);
		m->V[i + 2*nfaces] = GsPnt(r2*cos(angles[i]), r2*sin(angles[i]), d);
		m->V[i + 3*nfaces] = GsPnt(r1*cos(angles[i]), r1*sin(angles[i]), d);

	}

	angles.push_back(2*PI);

	for (int i = 0, j = 0; i < 2 * nfaces; i += 2, j++) {

		m->V[i + 4 * nfaces] = GsPnt(r1*cos(angles[j]), r1*sin(angles[j]), 0.0f);
		m->V[i + 1 + 4 * nfaces] = GsPnt(r3*cos((angles[j] + angles[j + 1]) / 2), r3*sin((angles[j] + angles[j + 1]) / 2), 0.0f);
		m->V[i + 6 * nfaces] = GsPnt(r1*cos(angles[j]), r1*sin(angles[j]), d);
		m->V[i + 1 + 6 * nfaces] = GsPnt(r3*cos((angles[j] + angles[j + 1]) / 2), r3*sin((angles[j] + angles[j + 1]) / 2), d);


	}

	// Faces

	//m->F.size(nfaces * 8);

	for (int i = 0; i < nfaces - 1; i++) {

		// Top and Bottom

		m->F.push().set(i, nfaces + i + 1, nfaces + i);
		m->F.push().set(i, i + 1, nfaces + i + 1);
		m->F.push().set(i + 2*nfaces, 3*nfaces + i, 3*nfaces + i + 1);
		m->F.push().set(i + 2*nfaces, 3*nfaces + i + 1, 2*nfaces + i + 1);

		// Inside

		m->F.push().set(i, 2 * nfaces + i, i + 1);
		m->F.push().set(2 * nfaces + i, 2 * nfaces + i + 1, i + 1);
		m->F.push().set(i + nfaces, i + 1 + nfaces, 3 * nfaces + i);
		m->F.push().set(3 * nfaces + i, i + 1 + nfaces, 3 * nfaces + i + 1);


	}

	// Last 2 triangles

	m->F.push().set(nfaces - 1, nfaces, 2*nfaces-1);
	m->F.push().set(nfaces - 1, 0, nfaces);
	m->F.push().set(3*nfaces - 1, 4*nfaces - 1, 3*nfaces);
	m->F.push().set(3*nfaces - 1, 3*nfaces, 2*nfaces);

	// TODO

	m->F.push().set(nfaces - 1, 3 * nfaces - 1, 0);
	m->F.push().set(3 * nfaces - 1, 2 * nfaces, 0);
	m->F.push().set(2 * nfaces - 1, nfaces, 4 * nfaces - 1);
	m->F.push().set(4 * nfaces - 1, nfaces, 3 * nfaces);


	// Edges

	for (int i = 0; i < nfaces*2-2; i+=2) {

		m->F.push().set(i + 4*nfaces, i + 4 * nfaces + 2, i + 4 * nfaces + 1);
		m->F.push().set(i + 6 * nfaces, i + 6 * nfaces + 1, i + 6 * nfaces + 2);

		m->F.push().set(i + 4 * nfaces, i + 4 * nfaces + 1, i + 6 * nfaces);
		m->F.push().set(i + 4 * nfaces + 1, i + 6 * nfaces + 1, i + 6 * nfaces);

		m->F.push().set(i + 4 * nfaces + 1, i + 4 * nfaces + 2, i + 6 * nfaces + 1);
		m->F.push().set(i + 4 * nfaces + 2, i + 6 * nfaces + 2, i + 6 * nfaces + 1);

		if (i == nfaces * 2 - 4) {
			m->F.push().set(i+2 + 4 * nfaces, 0 + 4*nfaces, i + 2 + 4 * nfaces + 1);
			m->F.push().set(i + 2 + 6 * nfaces, i + 2 + 6 * nfaces + 1, 0 + 6 * nfaces);

			m->F.push().set(i+2 + 4 * nfaces, i+2 + 4 * nfaces + 1, i + 2 + 6 * nfaces);
			m->F.push().set(i+2 + 4 * nfaces + 1, i+2 + 6 * nfaces + 1, i+2 + 6 * nfaces);

			m->F.push().set(i+2 + 4 * nfaces + 1, 0 + 4 * nfaces, i+2 + 6 * nfaces + 1);
			m->F.push().set(0 + 4 * nfaces, 0 + 6 * nfaces, i+2 + 6 * nfaces + 1);

		}

	}

	// Cross Edges



	m->compress();

}

void MyViewer::make_normal_gear_base(GsModel* m, float r, float d, int nfaces) {

	GsPnt a(0.0f, 0.0f, d/2);
	GsPnt b(0.0f, 0.0f, -d/2);

	m->make_cylinder(a, b, r, r, nfaces, false);

}

void MyViewer::make_normal_gear_edges(SnGroup* g, float r, float d, GsColor color, int nfaces) {

	GsPnt a(0.0f, 0.0f, d/2 - 0.0001f);
	GsPnt b(0.0f, 0.0f, -d/2 + 0.0001f);

	vector<float> angles;
	
	for (int i = 0; i < nfaces; i++) {

		angles.push_back(i * 2.0f * PI / nfaces);

	}

	vector<GsPnt> edgePts;

	for (int i = 0; i < nfaces; i++) {

		edgePts.push_back(GsPnt(r*cos(angles[i]), r*sin(angles[i]), 0));

	}

	SnModel* m;
	SnGroup* tempG;
	SnTransform* tempT;

	for (int i = 0; i < nfaces; i++) {

		m = new SnModel();
		m->model()->make_cylinder(a, b, r/10, r/10, nfaces, false);
		m->color(color);

		tempG = new SnGroup();
		tempG->separator(true);

		tempT = new SnTransform();
		tempG->add(tempT);
		tempG->add(m);
		tempT->get().translation(edgePts[i]);

		g->add(tempG);


	}

}

void MyViewer::make_line_gear(SnGroup *g, float length, float d, GsColor color, int nfaces) {
	SnModel *m;
	SnTransform *t1;
	SnTransform *t2;
	SnTransform *t3;
	SnGroup *g1;
	SnGroup *g2;
	SnGroup *g3;

	SnGroup *tempG;
	SnTransform *tempT;

	GsBox tempBox;

	tempBox.a = GsPnt(0.0f, 0.0f, 0.0f);
	tempBox.b = GsPnt(length, length, d);

	GsPnt a(0.0f, 0.0f, d / 2);
	GsPnt b(0.0f, 0.0f, -d / 2);


	for (int i = 0; i < nfaces; i++) {

		tempG = new SnGroup();
		tempG->separator(true);
		tempT = new SnTransform();
		tempG->add(tempT);

		//Box 1
		m = new SnModel();
		m->color(color);
		g1 = new SnGroup();
		g1->separator(true);
		t1 = new SnTransform();

		m->model()->make_box(tempBox);
		g1->add(t1);
		g1->add(m);
		t1->get().translation(0.0f, 0.0f, 0.0f);

		tempG->add(g1);

		//Box 2
		m = new SnModel();
		m->color(color);
		g2 = new SnGroup();
		g2->separator(true);
		t2 = new SnTransform();

		m->model()->make_box(tempBox);
		g2->add(t2);
		g2->add(m);
		t2->get().translation(length, 0.0f, 0.0f);

		tempG->add(g2);

		//Cylinder 3
		m = new SnModel();
		m->color(color);
		g3 = new SnGroup();
		g3->separator(true);
		t3 = new SnTransform();

		m->model()->make_cylinder(a, b, length / 2, length / 2, 20, true);
		g3->add(t3);
		g3->add(m);
		t3->get().translation((1.5f)*length, length, d / 2);

		tempG->add(g3);

		tempT->get().translation(i * 2 * length, 0.0f, 0.0f);

		g->add(tempG);
	}

}

void MyViewer::run_evaluation_test() {

	animationEval = true;
	double startTime;
	double finishTime;
	double completionTime;

	_model = new SnModel();
	SnGroup* g1 = new SnGroup();
	g1->separator(true);
	_model->color(GsColor::cyan);

	_t1 = new SnTransform();
	g1->add(_t1);

	startTime = gs_time();
	
	// Test 1
	// make_custom_gear(_model->model(), 0.2f, 0.4f, 100000);
	// g1->add(_model);

	// Test 2
	// make_custom_gear(_model->model(), 0.2f, 0.4f, 300000);
	// g1->add(_model);

	// Test 3
	// make_custom_gear(_model->model(), 0.2f, 0.4f, 500000);
	// g1->add(_model);

	// Test 4
	//make_normal_gear_base(_model->model(), 0.2f, 0.4f, 10);
	//g1->add(_model);
	//make_normal_gear_edges(g1, 0.2f, 0.4f, GsColor::red, 10);

	// Test 5
	//make_normal_gear_base(_model->model(), 0.2f, 0.4f, 50);
	//make_normal_gear_edges(g1, 0.2f, 0.4f, GsColor::red, 50);
	//g1->add(_model);

	// Test 6
	//make_normal_gear_base(_model->model(), 0.2f, 0.4f, 300);
	//make_normal_gear_edges(g1, 0.2f, 0.4f, GsColor::red, 300);
	//g1->add(_model);

	// Test 7
	//make_line_gear(g1, 0.2f, 0.2f, GsColor::red, 1000);

	// Test 8
	//make_line_gear(g1, 0.2f, 0.2f, GsColor::red, 5000);

	// Test 9
	//make_line_gear(g1, 0.2f, 0.2f, GsColor::red, 7500);

	
	finishTime = gs_time();
	completionTime = finishTime - startTime;
	cout << completionTime;


	_t1->get().translation(0.0f, 0.0f, 0.85f);

	rootg()->add(g1);

}

void MyViewer::build_scene ()
{

	double startTime;
	double finishTime;
	double completionTime;

	startTime = gs_time();

	
	// Create Sharp Gear 1
	_model = new SnModel();
	SnGroup* g1 = new SnGroup();
	g1->separator(true);

	_model->color(GsColor::cyan);
	make_custom_gear(_model->model(), 0.2f, 0.4f, 20);
	
	_t1 = new SnTransform();
	g1->add(_t1);
	g1->add(_model);
	_t1->get().translation(0.0f, 0.0f, 0.85f);


	// Create Normal Gear 2
	_model = new SnModel();
	SnGroup* g2 = new SnGroup();
	g2->separator(true);

	_model->color(GsColor::blue);
	make_normal_gear_base(_model->model(), 0.5, 0.4f, 20);

	_t2 = new SnTransform();
	g2->add(_t2);
	g2->add(_model);

	make_normal_gear_edges(g2, 0.5, 0.4f, GsColor::blue, 20);

	_t2->get().translation(0.0f, 0.0f, 0.0f);



	// Create Normal Gear 3
	_model = new SnModel();
	SnGroup* g3 = new SnGroup();
	g3->separator(true);

	_model->color(GsColor::red);
	make_normal_gear_base(_model->model(), 0.5, 0.4f, 20);

	_t3 = new SnTransform();
	g3->add(_t3);
	g3->add(_model);

	make_normal_gear_edges(g3, 0.5, 0.4f, GsColor::red, 20);

	_t3->get().translation(0.90f, 0.53f, 0.0f);



	// Create Normal Gear 4
	_model = new SnModel();
	SnGroup* g4 = new SnGroup();
	g4->separator(true);

	_model->color(GsColor::green);
	make_normal_gear_base(_model->model(), 0.5, 0.4f, 20);

	_t4 = new SnTransform();
	g4->add(_t4);
	g4->add(_model);

	make_normal_gear_edges(g4, 0.5, 0.4f, GsColor::green, 20);

	_t4->get().translation(-0.90f, -0.53f, 0.0f);


	// Create Pipe 5

	_model = new SnModel();
	SnGroup* g5 = new SnGroup();
	g5->separator(true);

	_model->color(GsColor::darkgreen);
	GsPnt a(0.0f, 0.0f, 0.0f);
	GsPnt b(0.0f, 0.0f, 1.50f);
	_model->model()->make_cylinder(a, b, 0.1f, 0.1f, 20, false);

	_t5 = new SnTransform();
	g5->add(_t5);
	g5->add(_model);

	_t5->get().translation(0.0f, 0.0f, -0.25f);


	// Create Pipe 6

	_model = new SnModel();
	SnGroup* g6 = new SnGroup();
	g6->separator(true);

	_model->color(GsColor::darkblue);
	a = GsPnt (0.0f, 0.0f, 0.0f);
	b = GsPnt (0.0f, 0.0f, -1.25f);
	_model->model()->make_cylinder(a, b, 0.1f, 0.1f, 20, false);

	_t6 = new SnTransform();
	g6->add(_t6);
	g6->add(_model);

	_t6->get().translation(0.90f, 0.53f, 0.25f);


	// Create Pipe 7

	_model = new SnModel();
	SnGroup* g7 = new SnGroup();
	g7->separator(true);

	_model->color(GsColor::darkblue);
	a = GsPnt(0.0f, 0.0f, 0.0f);
	b = GsPnt(0.0f, 0.0f, -1.25f);
	_model->model()->make_cylinder(a, b, 0.1f, 0.1f, 20, false);

	_t7 = new SnTransform();
	g7->add(_t7);
	g7->add(_model);

	_t7->get().translation(-0.90f, -0.53f, 0.25f);


	// Create Power Box 8

	_model = new SnModel();
	SnGroup* g8 = new SnGroup();
	g8->separator(true);

	_model->color(GsColor::darkgray);

	GsBox tempBox;
	tempBox.a = GsPnt(-0.15f, -0.15f, -0.15f);
	tempBox.b = GsPnt(0.15f, 0.15f, 0.15f);
	_model->model()->make_box(tempBox);

	_t8 = new SnTransform();
	g8->add(_t8);
	g8->add(_model);

	_t8->get().translation(0.90f, 0.53f, -1.16f);


	// Create Power Box 9

	_model = new SnModel();
	SnGroup* g9 = new SnGroup();
	g9->separator(true);

	_model->color(GsColor::darkgray);

	tempBox.a = GsPnt(-0.15f, -0.15f, -0.15f);
	tempBox.b = GsPnt(0.15f, 0.15f, 0.15f);
	_model->model()->make_box(tempBox);

	_t9 = new SnTransform();
	g9->add(_t9);
	g9->add(_model);

	_t9->get().translation(-0.90f, -0.53f, -1.16f);


	// Create Pipe 10

	_model = new SnModel();
	SnGroup* g10 = new SnGroup();
	g10->separator(true);

	_model->color(GsColor::darkred);
	a = GsPnt(0.0f, 0.15f, 0.0f);
	b = GsPnt(0.0f, 0.65f, 0.0f);
	_model->model()->make_cylinder(a, b, 0.1f, 0.1f, 20, false);

	_t10 = new SnTransform();
	g10->add(_t10);
	g10->add(_model);

	_t10->get().translation(-0.90f, -0.53f, -1.16f);


	// Create Pipe 11

	_model = new SnModel();
	SnGroup* g11 = new SnGroup();
	g11->separator(true);

	_model->color(GsColor::darkred);
	a = GsPnt(0.0f, 0.15f, 0.0f);
	b = GsPnt(0.0f, 0.65f, 0.0f);
	_model->model()->make_cylinder(a, b, 0.1f, 0.1f, 20, false);

	_t11 = new SnTransform();
	g11->add(_t11);
	g11->add(_model);

	_t11->get().translation(0.90f, 0.53f, -1.16f);



	// Create Sharp Gear 12
	_model = new SnModel();

	_model->color(GsColor::magenta);
	make_custom_gear(_model->model(), 0.2f, 0.2f, 10);

	SnGroup* g12a = new SnGroup();
	g12a->separator(true);
	_t12a = new SnTransform();
	g12a->add(_t12a);
	g12a->add(_model);
	_t12a->get().rotx(PI/2.0);

	SnGroup* g12b = new SnGroup();
	g12b->separator(true);
	_t12b = new SnTransform();
	g12b->add(_t12b);
	g12b->add(g12a);
	_t12b->get().translation(0.0f, 0.65f, 0.0f);

	SnGroup* g12c = new SnGroup();
	g12c->separator(true);
	_t12c = new SnTransform();
	g12c->add(_t12c);
	g12c->add(g12b);
	_t12c->get().translation(0.90f, 0.53f, -1.16f);


	// Create Sharp Gear 12
	_model = new SnModel();

	_model->color(GsColor::orange);
	make_custom_gear(_model->model(), 0.2f, 0.2f, 15);

	SnGroup* g13a = new SnGroup();
	g13a->separator(true);
	_t13a = new SnTransform();
	g13a->add(_t13a);
	g13a->add(_model);
	_t13a->get().rotx(PI / 2.0);

	SnGroup* g13b = new SnGroup();
	g13b->separator(true);
	_t13b = new SnTransform();
	g13b->add(_t13b);
	g13b->add(g13a);
	_t13b->get().translation(0.0f, 0.65f, 0.0f);

	SnGroup* g13c = new SnGroup();
	g13c->separator(true);
	_t13c = new SnTransform();
	g13c->add(_t13c);
	g13c->add(g13b);
	_t13c->get().translation(-0.90f, -0.53f, -1.16f);



	// Create Line Gear 14

	SnGroup* g14a = new SnGroup();
	g14a->separator(true);

	_t14a = new SnTransform();
	g14a->add(_t14a);

	make_line_gear(g14a, 0.1f, 0.2f, GsColor::brown, 10);

	_t14a->get().rotz(PI/2);

	SnGroup* g14b = new SnGroup();
	g14b->separator(true);

	_t14b = new SnTransform();
	g14b->add(_t14b);
	g14b->add(g14a);

	_t14b->get().translation(1.54f, -1.125f, 0.0f);



	// Create Line Gear 15

	SnGroup* g15a = new SnGroup();
	g15a->separator(true);

	_t15a = new SnTransform();
	g15a->add(_t15a);

	make_line_gear(g15a, 0.1f, 0.2f, GsColor::brown, 10);

	_t15a->get().rotz(-PI / 2);

	SnGroup* g15b = new SnGroup();
	g15b->separator(true);

	_t15b = new SnTransform();
	g15b->add(_t15b);
	g15b->add(g15a);

	_t15b->get().translation(-1.54f, 1.120f, 0.0f);



	// Roots
	SnGroup* gFinal = new SnGroup();
	gFinal->separator(true);

	_tFinal = new SnTransform();
	gFinal->add(_tFinal);

	gFinal->add(g2);
	gFinal->add(g3);
	gFinal->add(g4);
	gFinal->add(g5);
	gFinal->add(g6);
	gFinal->add(g7);
	gFinal->add(g8);
	gFinal->add(g9);
	gFinal->add(g1);
	gFinal->add(g10);
	gFinal->add(g11);
	gFinal->add(g12c);
	gFinal->add(g13c);
	gFinal->add(g14b);
	gFinal->add(g15b);

	_tFinal->get().roty(PI/4);

	rootg()->add(gFinal);

	finishTime = gs_time();
	completionTime = finishTime - startTime;
	cout << completionTime << endl;

}

// Below is an example of how to control the main loop of an animation:
void MyViewer::run_animation ()
{

	if (animationEval) {

		if (_animating) return; // avoid recursive calls
		_animating = true;

		int ind = gs_random(0, rootg()->size() - 1); // pick one child
		SnManipulator* manip = rootg()->get<SnManipulator>(ind); // access one of the manipulators
		GsMat m = manip->mat();

		_curang1 = 0;

		double frdt = 1.0 / 30.0; // delta time to reach given number of frames per second
		double v = 4; // target velocity is 1 unit per second
		double t = 0, lt = 0, t0 = gs_time();
		do // run for a while:
		{
			while (t - lt < frdt) t = gs_time() - t0; // wait until it is time for next frame
			lt = t;

			GsVec tr;
			_t1->get().getrans(tr);
			_t1->get().rotz(GS_TODEG(-_curang1));
			_t1->get().setrans(tr);


			_curang1 += 0.0005f;

			render(); // notify it needs redraw
			ws_check(); // redraw now
		} while (true);// m.e24 > 0 );
		_animating = false;

	}


	if (!animationEval) {

		if (_animating) return; // avoid recursive calls
		_animating = true;

		int ind = gs_random(0, rootg()->size() - 1); // pick one child
		SnManipulator* manip = rootg()->get<SnManipulator>(ind); // access one of the manipulators
		GsMat m = manip->mat();

		_curang1 = 0;

		double frdt = 1.0 / 30.0; // delta time to reach given number of frames per second
		double v = 4; // target velocity is 1 unit per second
		double t = 0, lt = 0, t0 = gs_time();
		do // run for a while:
		{
			while (t - lt < frdt) t = gs_time() - t0; // wait until it is time for next frame
			lt = t;

			GsVec tr;
			_t2->get().getrans(tr);
			_t2->get().rotz(GS_TODEG(-_curang1));
			_t2->get().setrans(tr);

			_t3->get().getrans(tr);
			_t3->get().rotz(GS_TODEG(_curang1));
			_t3->get().setrans(tr);

			_t4->get().getrans(tr);
			_t4->get().rotz(GS_TODEG(_curang1));
			_t4->get().setrans(tr);

			_t8->get().getrans(tr);
			_t8->get().rotz(GS_TODEG(_curang1));
			_t8->get().setrans(tr);

			_t9->get().getrans(tr);
			_t9->get().rotz(GS_TODEG(_curang1));
			_t9->get().setrans(tr);

			_t1->get().getrans(tr);
			_t1->get().rotz(GS_TODEG(-_curang1));
			_t1->get().setrans(tr);

			_t10->get().getrans(tr);
			_t10->get().rotz(GS_TODEG(_curang1));
			_t10->get().setrans(tr);

			_t11->get().getrans(tr);
			_t11->get().rotz(GS_TODEG(_curang1));
			_t11->get().setrans(tr);

			_t12b->get().getrans(tr);
			_t12b->get().roty(GS_TODEG(_curang1));
			_t12b->get().setrans(tr);

			_t12c->get().getrans(tr);
			_t12c->get().rotz(GS_TODEG(_curang1));
			_t12c->get().setrans(tr);

			_t13b->get().getrans(tr);
			_t13b->get().roty(GS_TODEG(-_curang1));
			_t13b->get().setrans(tr);

			_t13c->get().getrans(tr);
			_t13c->get().rotz(GS_TODEG(_curang1));
			_t13c->get().setrans(tr);

			_t14b->get().getrans(tr);
			tr += GsVec(0.0f, 0.018f, 0.0f);
			if (tr.y > 0.26) {
				tr.y = -1.125f;
			}

			_t14b->get().setrans(tr);


			_t15b->get().getrans(tr);
			tr -= GsVec(0.0f, 0.018f, 0.0f);
			if (tr.y < -0.26) {
				tr.y = 1.120f;
			}

			_t15b->get().setrans(tr);


			_curang1 += 0.0005f;

			render(); // notify it needs redraw
			ws_check(); // redraw now
		} while (true);// m.e24 > 0 );
		_animating = false;

	}

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
		case 'n' : { bool b=!_nbut->value(); _nbut->value(b); show_normals(b); return 1; }
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
