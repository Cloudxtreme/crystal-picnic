using System;
using System.Drawing;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Collections.Generic;

struct Key_Info {
	public bool Control;
	//public bool Alt;
	public bool Shift;
	public Keys KeyCode;
};

class Parent_Window : Form {
	public Main_Window mw;
	public Tool_Window tlw;
	private MainMenu mainMenu;
	private MenuItem layers_menu;
	public static List<MenuItem> layer_items;

	public void recreate_layers_menu() {
		layers_menu.MenuItems.Clear();
		layer_items.Clear();
		for (int i = 0; i < Main_Panel.layers.Count; i++) {
			MenuItem m = new MenuItem("Show Layer " + i, new EventHandler(delegate(object sender, EventArgs e) {
				((MenuItem)sender).Checked = !((MenuItem)sender).Checked;
				mw.panel.Invalidate();
			}), Shortcut.None);
			m.Checked = true;
			layers_menu.MenuItems.Add(m);
			layer_items.Add(m);
		}
	}

	public Parent_Window() {
		this.IsMdiContainer = true;

		Text = "AshEdit2.5";

		ClientSize = new Size(800, 500);

		mainMenu = new MainMenu();
		MenuItem File = mainMenu.MenuItems.Add("&File");
        File.MenuItems.Add(new MenuItem("&New", new EventHandler(delegate(object sender, EventArgs e)
        {
            mw.panel.new_map();
        }), Shortcut.CtrlN));
        File.MenuItems.Add(new MenuItem("&Open", new EventHandler(delegate(object sender, EventArgs e)
        {
			mw.panel.load();
		}), Shortcut.CtrlO));
		File.MenuItems.Add(new MenuItem("&Save",new EventHandler(delegate(object sender, EventArgs e) {
			mw.panel.save();
		}), Shortcut.CtrlS));
		File.MenuItems.Add(new MenuItem("-"));
		File.MenuItems.Add(new MenuItem("Save screenshot",new EventHandler(delegate(object sender, EventArgs e) {
			int w, h;
			mw.panel.get_draw_size(out w, out h);
			Bitmap b = new Bitmap(w, h);
			Graphics g = Graphics.FromImage(b);
			g.Clear(Color.FromArgb(0));
			mw.panel.draw(g, true);
			SaveFileDialog sfd = new SaveFileDialog();
			sfd.DefaultExt = "";
			sfd.OverwritePrompt = true;
			DialogResult dr = sfd.ShowDialog();
			if (dr == DialogResult.OK) {
				b.Save(sfd.FileName);
			}
			g.Dispose();
		}), Shortcut.None));
		File.MenuItems.Add(new MenuItem("-"));
		File.MenuItems.Add(new MenuItem("Load heightmap",new EventHandler(delegate(object sender, EventArgs e) {
			mw.panel.load_heightmap();
		}), Shortcut.None));
		File.MenuItems.Add(new MenuItem("Save heightmap",new EventHandler(delegate(object sender, EventArgs e) {
			mw.panel.save_heightmap();
		}), Shortcut.None));
		File.MenuItems.Add(new MenuItem("-"));
		File.MenuItems.Add(new MenuItem("&Exit",new EventHandler(this.FileExit_clicked),Shortcut.CtrlQ));
		MenuItem Edit = mainMenu.MenuItems.Add("&Edit");
		Edit.MenuItems.Add(new MenuItem("&Undo",new EventHandler(delegate(object sender, EventArgs e) {
			mw.panel.undo();
		}), Shortcut.CtrlZ));
		Edit.MenuItems.Add(new MenuItem("&Redo",new EventHandler(delegate(object sender, EventArgs e) {
			mw.panel.redo();
		}), Shortcut.CtrlShiftZ));
		MenuItem Size = mainMenu.MenuItems.Add("&Size");
		MenuItem mi;
		mi = new MenuItem("Insert row before cursor", new EventHandler(delegate(object sender, EventArgs e) {
			Key_Info ki = new Key_Info();
			ki.KeyCode = Keys.R;
			ki.Control = true;
			mw.panel.handle_key(ki);
		}), Shortcut.CtrlR);
		mi.Visible = false;
		Size.MenuItems.Add(mi);
		mi = new MenuItem("Insert row after cursor", new EventHandler(delegate(object sender, EventArgs e) {
			Key_Info ki = new Key_Info();
			ki.KeyCode = Keys.R;
			ki.Shift = true;
			ki.Control = true;
			mw.panel.handle_key(ki);
		}), Shortcut.CtrlShiftR);
		mi.Visible = false;
		Size.MenuItems.Add(mi);
		mi = new MenuItem("Insert column before cursor", new EventHandler(delegate(object sender, EventArgs e) {
			Key_Info ki = new Key_Info();
			ki.KeyCode = Keys.C;
			ki.Control = true;
			mw.panel.handle_key(ki);
		}), Shortcut.CtrlC);
		mi.Visible = false;
		Size.MenuItems.Add(mi);
		mi = new MenuItem("Insert column after cursor", new EventHandler(delegate(object sender, EventArgs e) {
			Key_Info ki = new Key_Info();
			ki.KeyCode = Keys.C;
			ki.Shift = true;
			ki.Control = true;
			mw.panel.handle_key(ki);
		}), Shortcut.CtrlShiftC);
		mi.Visible = false;
		Size.MenuItems.Add(mi);
		mi = new MenuItem("Delete row", new EventHandler(delegate(object sender, EventArgs e) {
			Key_Info ki = new Key_Info();
			ki.KeyCode = Keys.D;
			ki.Control = true;
			mw.panel.handle_key(ki);
		}), Shortcut.CtrlD);
		mi.Visible = false;
		Size.MenuItems.Add(mi);
		mi = new MenuItem("Delete column", new EventHandler(delegate(object sender, EventArgs e) {
			Key_Info ki = new Key_Info();
			ki.KeyCode = Keys.D;
			ki.Shift = true;
			ki.Control = true;
			mw.panel.handle_key(ki);
		}), Shortcut.CtrlShiftD);
		mi.Visible = false;
		Size.MenuItems.Add(mi);
		Size.MenuItems.Add(new MenuItem("Insert layer before current layer", new EventHandler(delegate(object sender, EventArgs e) {
			Key_Info ki = new Key_Info();
			ki.KeyCode = Keys.L;
			ki.Control = true;
			mw.panel.handle_key(ki);
			tlw.Invalidate();
			recreate_layers_menu();
		}), Shortcut.CtrlL));
		Size.MenuItems.Add(new MenuItem("Insert layer after current layer", new EventHandler(delegate(object sender, EventArgs e) {
			Key_Info ki = new Key_Info();
			ki.KeyCode = Keys.L;
			ki.Shift = true;
			ki.Control = true;
			mw.panel.handle_key(ki);
			tlw.Invalidate();
			recreate_layers_menu();
		}), Shortcut.CtrlShiftL));
		Size.MenuItems.Add(new MenuItem("Delete current layer", new EventHandler(delegate(object sender, EventArgs e) {
			Key_Info ki = new Key_Info();
			ki.KeyCode = Keys.X;
			ki.Control = true;
			mw.panel.handle_key(ki);
			tlw.Invalidate();
			recreate_layers_menu();
		}), Shortcut.CtrlX));
		layers_menu = mainMenu.MenuItems.Add("&Layers");
		layer_items = new List<MenuItem>();
		MenuItem Arrange = mainMenu.MenuItems.Add("&Window");
		Arrange.MenuItems.Add(new MenuItem("&Cascade",new EventHandler(this.Cascade_clicked),Shortcut.F1));
		Arrange.MenuItems.Add(new MenuItem("&Horizontal",new EventHandler(this.Horizontal_clicked),Shortcut.F2));
		Arrange.MenuItems.Add(new MenuItem("&Vertical",new EventHandler(this.Vertical_clicked),Shortcut.F3));
		Arrange.MenuItems.Add(new MenuItem("-"));
		Arrange.MenuItems.Add(new MenuItem("&Main Window",new EventHandler(delegate(object sender, EventArgs ea) {
			Entry_Point.mw.Visible = true;
		}),Shortcut.CtrlM));
		Arrange.MenuItems.Add(new MenuItem("&Tile Window",new EventHandler(delegate(object sender, EventArgs ea) {
			Entry_Point.tw.Visible = true;
		}),Shortcut.CtrlT));
		Arrange.MenuItems.Add(new MenuItem("Tool&s Window",new EventHandler(delegate(object sender, EventArgs ea) {
			Entry_Point.tlw.Visible = true;
		}),Shortcut.CtrlShiftT));
		MenuItem Help = mainMenu.MenuItems.Add("&Help");
		Help.MenuItems.Add(new MenuItem("Reference", new EventHandler(delegate(object sender, EventArgs e) {
			Form f = new Form();
			f.Size = new Size(400, 400);
			Label l = new Label();
			l.Text = "Insert row before cursor - Ctrl-R\nInsertrow after cursor - Ctrl-Shift-R\nInsert column before cursor - Ctrl-C\nInsert column after cursor - Ctrl-Shift-C\nDelete row - Ctrl-D\nDeleteColumn - Ctrl-Shift-D\nInsert layer before current layer - Ctrl-L\nInsert layer after current layer - Ctrl-Shift-L\nDelete current layer - Ctrl-X\nUndo - Ctrl-Z\nRedo - Ctrl-Shift-Z\n\n\n\nVersion 0.11";
			l.Location = new Point(0, 0);
			l.Size = new Size(390, 390);
			l.Parent = f;
			f.ShowDialog();
		}), Shortcut.F1));
		this.Menu=mainMenu;
   
		CenterToScreen();
	}
	
	private void FileExit_clicked(object sender, EventArgs e) {
		this.Close();
	}

	private void Cascade_clicked(object sender, EventArgs e) {
		this.LayoutMdi(MdiLayout.Cascade );
	}

	private void Horizontal_clicked(object sender, EventArgs e) {
		this.LayoutMdi(MdiLayout.TileHorizontal);
	}

	private void Vertical_clicked(object sender, EventArgs e) {
		this.LayoutMdi(MdiLayout.TileVertical);
	}
}

class Entry_Point {
	public static Parent_Window pw;
	public static Tile_Window tw;
	public static Main_Window mw;
	public static Tool_Window tlw;

    [STAThread]
	public static void Main() {
		pw = new Parent_Window();

		tw = new Tile_Window(pw);
		tw.Text = "Tiles";
		tw.MdiParent = pw;;
		tw.Visible = true;

		mw = new Main_Window(pw, tw);
		mw.Text = "Area";
		mw.MdiParent = pw;
		mw.Visible = true;

		tlw = new Tool_Window(mw, pw, tw);
		tlw.Text = "Tools";
		tlw.MdiParent = pw;
		tlw.Visible = true;

		pw.mw = mw;
		pw.tlw = tlw;

		Application.Run(pw);
	}
}
