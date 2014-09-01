using System;
using System.Drawing;
using System.Windows.Forms;
using System.Collections.Generic;
using System.Drawing.Imaging;

class Tool_Window : Form {
	private Main_Window mw;
	private Parent_Window pw;
	private Tile_Window tw;

	private Label sheet_label;
	public static Label layer_label;
    private Label tool_label;

    public static Label src_layer_label;
    private Button prev_src_layer, next_src_layer;

	public static int curr_height = 0;
	private Label height_num_label;

	public enum Tool_Type {
		PAINT = 0,
		CLEAR,
		SOLID,
		FILL,
		CLONE,
		HEIGHT_FILL,
		HEIGHT_PAINT,
        LAYER_PAINT,
        INFO
	};
	public static Tool_Type[] tools = {
		Tool_Type.PAINT,
		Tool_Type.CLEAR,
		Tool_Type.SOLID,
		Tool_Type.FILL,
		Tool_Type.CLONE,
		Tool_Type.HEIGHT_FILL,
		Tool_Type.HEIGHT_PAINT,
        Tool_Type.LAYER_PAINT,
        Tool_Type.INFO
	};
	string[] tool_names = {
		"Paint",
		"Clear",
		"Solid",
		"Fill",
		"Clone",
		"Height Fill",
		"Height Paint",
        "Layer Paint",
        "Info"
	};
	public static int curr_tool = 0;

	public static RadioButton fill_all;
	public RadioButton fill_layer;
	
	public Tool_Window(Main_Window mw, Parent_Window pw, Tile_Window tw) {
		this.mw = mw;
		this.pw = pw;
		this.tw = tw;

		Width = 200;
		Height = 400;
		FormBorderStyle = FormBorderStyle.FixedSingle;

		Button prev_sheet = new Button();
		prev_sheet.Image = new Bitmap("arrow_left.png");
		prev_sheet.Parent = this;
		prev_sheet.Location = new Point(5, 5);
		prev_sheet.Width = 20;
		prev_sheet.Height = 20;
		prev_sheet.Click += new EventHandler(delegate(object sender, EventArgs ea) {
			if (Tile_Panel.curr_sheet > 0) {
				Tile_Panel.curr_sheet--;
				tw.panel.Invalidate();
				sheet_label.Text = "Sheet " + Tile_Panel.curr_sheet;
			}
		});
		Button next_sheet = new Button();
		next_sheet.Image = new Bitmap("arrow_right.png");
		next_sheet.Parent = this;
		next_sheet.Location = new Point(140, 5);
		next_sheet.Width = 20;
		next_sheet.Height = 20;
		next_sheet.Click += new EventHandler(delegate(object sender, EventArgs ea) {
			if (Tile_Panel.curr_sheet < Tile_Panel.tile_sheets.Count-1) {
				Tile_Panel.curr_sheet++;
				tw.panel.Invalidate();
				sheet_label.Text = "Sheet " + Tile_Panel.curr_sheet;
			}
		});
		sheet_label = new Label();
		sheet_label.Text = "Sheet 0";
		sheet_label.Parent = this;
		sheet_label.Location = new Point(35, 5);

        prev_src_layer = new Button();
        prev_src_layer.Image = new Bitmap("arrow_left.png");
        prev_src_layer.Parent = this;
        prev_src_layer.Location = new Point(5, 30);
        prev_src_layer.Width = 20;
        prev_src_layer.Height = 20;
        prev_src_layer.Click += new EventHandler(delegate(object sender, EventArgs ea)
        {
            if (Main_Panel.src_layer > 0)
            {
                Main_Panel.src_layer--;
                src_layer_label.Text = "Source layer " + Main_Panel.src_layer;
                Main_Panel.layer_painted.Clear();
                mw.panel.Invalidate();
            }
        });
        next_src_layer = new Button();
        next_src_layer.Image = new Bitmap("arrow_right.png");
        next_src_layer.Parent = this;
        next_src_layer.Location = new Point(140, 30);
        next_src_layer.Width = 20;
        next_src_layer.Height = 20;
        next_src_layer.Click += new EventHandler(delegate(object sender, EventArgs ea)
        {
            if (Main_Panel.src_layer < Main_Panel.layers.Count - 1)
            {
                Main_Panel.src_layer++;
                src_layer_label.Text = "Source layer " + Main_Panel.src_layer;
                Main_Panel.layer_painted.Clear();
                mw.panel.Invalidate();
            }
        });
        src_layer_label = new Label();
        src_layer_label.Text = "Source Layer 0";
        src_layer_label.Parent = this;
        src_layer_label.Location = new Point(35, 30);

        disable_src_layer();

		Button prev_layer = new Button();
		prev_layer.Image = new Bitmap("arrow_left.png");
		prev_layer.Parent = this;
		prev_layer.Location = new Point(5, 55);
		prev_layer.Width = 20;
		prev_layer.Height = 20;
		prev_layer.Click += new EventHandler(delegate(object sender, EventArgs ea) {
			if (Main_Panel.layer > 0) {
				Main_Panel.layer--;
				layer_label.Text = "Layer " + Main_Panel.layer;
                Main_Panel.layer_painted.Clear();
                mw.panel.Invalidate();
            }
		});
		Button next_layer = new Button();
		next_layer.Image = new Bitmap("arrow_right.png");
		next_layer.Parent = this;
		next_layer.Location = new Point(140, 55);
		next_layer.Width = 20;
		next_layer.Height = 20;
		next_layer.Click += new EventHandler(delegate(object sender, EventArgs ea) {
			if (Main_Panel.layer < Main_Panel.layers.Count-1) {
				Main_Panel.layer++;
				layer_label.Text = "Layer " + Main_Panel.layer;
                Main_Panel.layer_painted.Clear();
                mw.panel.Invalidate();
            }
		});
		layer_label = new Label();
		layer_label.Text = "Layer 0";
		layer_label.Parent = this;
		layer_label.Location = new Point(35, 55);

		Button prev_tool = new Button();
		prev_tool.Image = new Bitmap("arrow_left.png");
		prev_tool.Parent = this;
		prev_tool.Location = new Point(5, 80);
		prev_tool.Width = 20;
		prev_tool.Height = 20;
		prev_tool.Click += new EventHandler(delegate(object sender, EventArgs ea) {
			if (curr_tool > 0) {
				curr_tool--;
				tool_label.Text = tool_names[curr_tool];
                if (tool_label.Text == "Layer Paint")
                {
                    enable_src_layer();
                }
                else
                {
                    disable_src_layer();
                }
                Main_Panel.layer_painted.Clear();
                mw.panel.Invalidate();
            }
		});
		Button next_tool = new Button();
		next_tool.Image = new Bitmap("arrow_right.png");
		next_tool.Parent = this;
		next_tool.Location = new Point(140, 80);
		next_tool.Width = 20;
		next_tool.Height = 20;
		next_tool.Click += new EventHandler(delegate(object sender, EventArgs ea) {
			if (curr_tool < tools.Length-1) {
				curr_tool++;
				tool_label.Text = tool_names[curr_tool];
                if (tool_label.Text == "Layer Paint")
                {
                    enable_src_layer();
                }
                else
                {
                    disable_src_layer();
                }
                Main_Panel.layer_painted.Clear();
                mw.panel.Invalidate();
            }
		});
		tool_label = new Label();
		tool_label.Text = "Paint";
		tool_label.Parent = this;
		tool_label.Location = new Point(35, 80);

		CheckBox show_grid = new CheckBox();
		show_grid.Text = "Show Grid";
		show_grid.Checked = true;
		show_grid.Click += new EventHandler(delegate(object sender, EventArgs ea) {
			mw.panel.show_grid = show_grid.Checked;
			mw.panel.Invalidate();
		});
		show_grid.Parent = this;
		show_grid.Location = new Point(5, 110);

        CheckBox show_tile = new CheckBox();
        show_tile.Text = "Show Tile at Cursor";
        show_tile.Checked = false;
        show_tile.Click += new EventHandler(delegate(object sender, EventArgs ea)
        {
            mw.panel.show_tile = show_tile.Checked;
            mw.panel.Invalidate();
        });
        show_tile.Parent = this;
        show_tile.Location = new Point(5, 135);
        show_tile.Width = 300;

        CheckBox show_outline = new CheckBox();
        show_outline.Text = "Show Outline at Cursor";
        show_outline.Checked = false;
        show_outline.Click += new EventHandler(delegate(object sender, EventArgs ea)
        {
            mw.panel.show_outline = show_outline.Checked;
            mw.panel.Invalidate();
        });
        show_outline.Parent = this;
        show_outline.Location = new Point(5, 160);
        show_outline.Width = 300;
		
		Label fill_label = new Label();
		fill_label.Text = "Fill test:";
		fill_label.Parent = this;
		fill_label.Location = new Point(5, 195);

		Panel fill_panel = new Panel();
		fill_panel.Parent = this;
		fill_panel.Location = new Point(5, 215);
		fill_panel.Size = new Size(165, 40);

		fill_all = new RadioButton();
		fill_all.Text = "All layers";
		fill_all.Parent = fill_panel;
		fill_all.Location = new Point(20, 0);
		fill_all.Checked = true;
		fill_layer = new RadioButton();
		fill_layer.Text = "Current layer";
		fill_layer.Parent = fill_panel;
		fill_layer.Location = new Point(20, 20);
		
		Label height_label = new Label();
		height_label.Text = "Height:";
		height_label.Parent = this;
		height_label.Location = new Point(5, 255);
		
		Button prev_height = new Button();
		prev_height.Image = new Bitmap("arrow_left.png");
		prev_height.Parent = this;
		prev_height.Location = new Point(5, 285);
		prev_height.Width = 20;
		prev_height.Height = 20;
		prev_height.Click += new EventHandler(delegate(object sender, EventArgs ea) {
			if (curr_height > -7) {
				curr_height--;
				height_num_label.Text = curr_height.ToString();
			}
		});
		Button next_height = new Button();
		next_height.Image = new Bitmap("arrow_right.png");
		next_height.Parent = this;
		next_height.Location = new Point(140, 285);
		next_height.Width = 20;
		next_height.Height = 20;
		next_height.Click += new EventHandler(delegate(object sender, EventArgs ea) {
			if (curr_height < 8) {
				curr_height++;
				height_num_label.Text = curr_height.ToString();
			}
		});
		
		height_num_label = new Label();
		height_num_label.Text = "0";
		height_num_label.Parent = this;
		height_num_label.Location = new Point(35, 285);
		
		CheckBox show_height = new CheckBox();
		show_height.Text = "Show Height";
		show_height.Checked = false;
		show_height.Click += new EventHandler(delegate(object sender, EventArgs ea) {
			mw.panel.show_height = show_height.Checked;
			mw.panel.Invalidate();
		});
		show_height.Parent = this;
		show_height.Location = new Point(5, 310);
	}

	protected override void OnFormClosing(FormClosingEventArgs e) {
		if (e.CloseReason == CloseReason.MdiFormClosing) {
			return;
		}
		Visible = false;
		e.Cancel = true;
	}

	protected override void OnInvalidated(InvalidateEventArgs e) {
		layer_label.Text = "Layer " + Main_Panel.layer;
	}

    private void disable_src_layer()
    {
        src_layer_label.Enabled = false;
        prev_src_layer.Enabled = false;
        next_src_layer.Enabled = false;
    }

    private void enable_src_layer()
    {
        src_layer_label.Enabled = true;
        prev_src_layer.Enabled = true;
        next_src_layer.Enabled = true;
    }
}
