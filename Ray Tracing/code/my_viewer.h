# pragma once

# include <sig/sn_poly_editor.h>
# include <sig/sn_lines2.h>

# include <sigogl/ui_button.h>
# include <sigogl/ws_viewer.h>

// Viewer for this example:
class MyViewer : public WsViewer
{  protected :
	enum MenuEv { EvRegenerate, EvRayTrace, EvCreateGIF, EvExit };
	UiCheckButton* _nbut;
   public :
	MyViewer ( int x, int y, int w, int h, const char* l );
	void add_ui ();
	void build_scene ();
	void add_model(SnShape* s, GsVec p);
	virtual int handle_keyboard ( const GsEvent &e ) override;
	virtual int uievent ( int e ) override;
};

