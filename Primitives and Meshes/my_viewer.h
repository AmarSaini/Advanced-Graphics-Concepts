# pragma once

# include <sig/sn_poly_editor.h>
# include <sig/sn_lines2.h>

# include <sigogl/ui_button.h>
# include <sigogl/ws_viewer.h>
# include <sig/sn_model.h>

// Viewer for this example:
class MyViewer : public WsViewer
{  protected :
	enum MenuEv { EvNormals, EvAnimate, EvExit };
	UiCheckButton* _nbut;
	bool _animating;
	float _curang1;
	SnModel* _model;
	SnTransform* _t1;
	SnTransform* _t2;
	SnTransform* _t3;
	SnTransform* _t4;
	SnTransform* _t5;
	SnTransform* _t6;
	SnTransform* _t7;
	SnTransform* _t8;
	SnTransform* _t9;
	SnTransform* _t10;
	SnTransform* _t11;
	SnTransform* _t12a;
	SnTransform* _t12b;
	SnTransform* _t12c;
	SnTransform* _t13a;
	SnTransform* _t13b;
	SnTransform* _t13c;
	SnTransform* _t14a;
	SnTransform* _t14b;
	SnTransform* _t15a;
	SnTransform* _t15b;
	SnTransform* _tFinal;

	bool animationEval = false;


   public :
	MyViewer ( int x, int y, int w, int h, const char* l );
	void add_ui ();
	void add_model ( SnShape* s, GsVec p );
	void make_custom_gear(GsModel* m, float r1, float d, int nfaces);
	void make_normal_gear_base(GsModel* m, float r, float d, int nfaces);
	void make_normal_gear_edges(SnGroup* g, float r, float d, GsColor color, int nfaces);
	void make_line_gear(SnGroup *g, float length, float d, GsColor color, int nfaces);
	void run_evaluation_test();
	void build_scene ();
	void show_normals ( bool b );
	void run_animation ();
	virtual int handle_keyboard ( const GsEvent &e ) override;
	virtual int uievent ( int e ) override;
};

