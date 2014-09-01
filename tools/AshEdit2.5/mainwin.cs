using System;
using System.Drawing;
using System.Windows.Forms;
using System.Collections.Generic;
using System.Drawing.Imaging;
using System.IO;
using System.Threading;
using System.Runtime.InteropServices;

class Tile {
	public bool solid;
	public int tile_sheet;
	public int tile_num;

	public Tile() {
		solid = false;
		tile_sheet = 0;
		tile_num = -1;
	}

	public Tile(Tile t) {
		solid = t.solid;
		tile_sheet = t.tile_sheet;
		tile_num = t.tile_num;
	}
}

struct Layer {
	public List<List<Tile>> tiles;
}

class Main_Panel : Panel {
	private Point[] directions = new Point[] {
		new Point(-1, 1),
		new Point(1, -1),
		new Point(-2, -2),
		new Point(2, 2)
	};

    Bitmap tmp_bmp = new Bitmap(General.TILE_SIZE, General.TILE_SIZE, PixelFormat.Format32bppArgb);
    Bitmap tinted = new Bitmap(General.TILE_SIZE, General.TILE_SIZE, PixelFormat.Format32bppArgb);

	List<List<int>> heightmap;

    public static List<Point> layer_painted = new List<Point>();

	public bool show_grid = true;
    public bool show_tile = false;
    public bool show_outline = false;
    public bool show_height = false;
	
	private bool mouse_down = false;
	private bool first_down = false;

    private static Size default_size = new Size(20, 20);
	public static Size size = default_size;
	private Point top = new Point(0, 0);

	public static int layer = 0;
    public static int src_layer = 0;
    public static List<Layer> layers;

	private const int MAX_UNDO = 10;
	private static List<List<Layer>> undo_stack = new List<List<Layer>>();
	private static List<List<Layer>> redo_stack = new List<List<Layer>>();
	private static List<List<List<int>>> heightmap_undo_stack = new List<List<List<int>>>();
	private static List<List<List<int>>> heightmap_redo_stack = new List<List<List<int>>>();

	private int mouse_x = 0, mouse_y = 0;

	private Point clone_down_tile;
	private Point clone_down_iso;
	private Point clone_down_screen;
	private Point clone_prev_iso;

	private Tile_Window tw;
	private Parent_Window pw;

	private Bitmap height_bmp;

	public Main_Panel(Parent_Window pw, Tile_Window tw) {
		this.pw = pw;
		this.tw = tw;

		AutoScroll = true;
		Dock = DockStyle.Fill;

		init_layers(size.Width, size.Height);
		init_heightmap(size.Width, size.Height);

		resize();

		DoubleBuffered = true;

		// Make it accept input
		this.SetStyle(ControlStyles.Selectable, true);
		this.TabStop = true;

		/*
		Thread thr = new Thread(invalidator);
		thr.Start();
		*/

        Bitmap tmp = new Bitmap("heights.png");
        height_bmp = new Bitmap(tmp.Width, tmp.Height, PixelFormat.Format32bppPArgb);
        Graphics hg = Graphics.FromImage(height_bmp);
        hg.DrawImage(tmp, new Point(0, 0));
        hg.Dispose();
        tmp.Dispose();
		
		pw.recreate_layers_menu();
	}

    public void new_map()
    {
        DialogResult dr = MessageBox.Show("Erase this level and start a new one?", "Warning", MessageBoxButtons.YesNo);
        if (dr == DialogResult.No)
        {
            return;
        }
        size = default_size;
        layer = 0;
        Tool_Window.layer_label.Text = "Layer 0";
        HorizontalScroll.Value = 0;
        VerticalScroll.Value = 0;
        init_layers(size.Width, size.Height);
        init_heightmap(size.Width, size.Height);
        resize();
        pw.recreate_layers_menu();
        Invalidate();
    }

	void init_layers(int w, int h) {
		layers = new List<Layer>();

		Layer layer0 = new Layer();
		layer0.tiles = new List<List<Tile>>();
		for (int i = 0; i < h; i++) {
			List<Tile> l = new List<Tile>();
			for (int j = 0; j < w; j++) {
				Tile t = new Tile();
				l.Add(t);
			}
			layer0.tiles.Add(l);
		}
		layers.Add(layer0);
	}

	void init_heightmap(int w, int h) {
		heightmap = new List<List<int>>();

		for (int i = 0; i < h; i++) {
			List<int> l = new List<int>();
			for (int j = 0; j < w; j++) {
				l.Add(0);
			}
			heightmap.Add(l);
		}
	}

	private static void clone_map(List<Layer> cloning, List<List<int>> cloning_heightmap, out List<Layer> cloned, out List<List<int>> out_heightmap) {
		cloned = new List<Layer>();
		out_heightmap = new List<List<int>>();
		for (int ll = 0; ll < cloning.Count; ll++) {
			Layer layer0 = new Layer();
			layer0.tiles = new List<List<Tile>>();
			for (int i = 0; i < cloning[0].tiles.Count; i++) {
				List<Tile> l = new List<Tile>();
				List<int> h = new List<int>();
				for (int j = 0; j < cloning[0].tiles[0].Count; j++) {
					Tile t = new Tile(cloning[ll].tiles[i][j]);
					l.Add(t);
					if (ll == 0) {
						h.Add(cloning_heightmap[i][j]);
					}
				}
				layer0.tiles.Add(l);
				if (ll == 0) {
					out_heightmap.Add(h);
				}
			}
			cloned.Add(layer0);
		}
	}

	public void get_draw_size(out int w, out int h) {
		w = size.Width*General.TILE_SIZE;
		h = size.Height*General.TILE_SIZE;
		int startx = w;
		int starty = 0;
		General.iso_project(ref w, ref starty);
		General.iso_project(ref startx, ref h);
	}

	public void resize() {
		General.iso_offset = new Point((size.Width > size.Height ? size.Width : size.Height) *General.TILE_SIZE / 2, 0);
		int w, h;
		get_draw_size(out w, out h);
		AutoScrollMinSize = new Size(w, h);;
	}

	private void save_undo(List<Layer> clone, List<List<int>> clone_heightmap) {
		if (undo_stack.Count >= MAX_UNDO) {
			undo_stack.RemoveAt(MAX_UNDO-1);
			heightmap_undo_stack.RemoveAt(MAX_UNDO-1);
		}
		undo_stack.Insert(0, clone);
		heightmap_undo_stack.Insert(0, clone_heightmap);
		redo_stack.Clear();
	}

	private void save_undo() {
		List<Layer> map;
		List<List<int>> heightmap_clone;
		clone_map(layers, heightmap, out map, out heightmap_clone);
		save_undo(map, heightmap_clone);
	}
	
	protected override void OnPaint(PaintEventArgs pea) {
		draw(pea.Graphics, false);
	}

	public unsafe void draw(Graphics g, bool screenshot) {
		Pen pen = new Pen(Color.Black);
        Pen p_yellow = new Pen(Color.Yellow);
        Pen p_magenta = new Pen(Color.Magenta);
        int hoffs;
		int voffs;

		if (screenshot) {
			hoffs = 0;
			voffs = 0;
		}
		else {
			hoffs = HorizontalScroll.Value;
			voffs = VerticalScroll.Value;
		}

		for (int i = 0; i < layers.Count; i++) {
			if (Parent_Window.layer_items[i].Checked == false)
				continue;
			Layer l = layers[i];
			for (int y = 0; y < size.Height; y++) {
				for (int x = 0; x < size.Width; x++) {
					int dx = x*General.TILE_SIZE;
					int dy = y*General.TILE_SIZE;
					General.iso_project(ref dx, ref dy);
					dx -= General.TILE_SIZE/2;
					dy -= General.TILE_SIZE/2;
					Tile t = l.tiles[y][x];
					if (t.tile_num != -1) {
						Bitmap img = Tile_Panel.tile_sheets[t.tile_sheet];
						int x1 = t.tile_num % (1024/General.TILE_SIZE);
						int y1 = t.tile_num / (1024/General.TILE_SIZE);
						x1 *= General.TILE_SIZE;
						y1 *= General.TILE_SIZE;
						int x2 = x1 + General.TILE_SIZE;
						int y2 = y1 + General.TILE_SIZE;
						dx -= hoffs;
						dy -= voffs;
                        Point p = new Point(x, y);
                        if (layer != i || layer_painted.BinarySearch(p, new C()) < 0)
                        {
                            g.DrawImage(img, new Point[] { new Point(dx, dy), new Point(dx + General.TILE_SIZE, dy), new Point(dx, dy + General.TILE_SIZE) },
                                new Rectangle(x1, y1, x2 - x1, y2 - y1), GraphicsUnit.Pixel, new ImageAttributes(), null
                            );
                        }
                        else
                        {
                            Graphics tg = Graphics.FromImage(tmp_bmp);
                            tg.Clear(Color.Transparent);
                            tg.DrawImage(img, new Point[] { new Point(0, 0), new Point(General.TILE_SIZE, 0), new Point(0, General.TILE_SIZE) },
                                new Rectangle(x1, y1, x2 - x1, y2 - y1), GraphicsUnit.Pixel, new ImageAttributes(), null
                            );
                            tg.Dispose();
                            Rectangle src_rect = new Rectangle(0, 0, 64, 64);
                            Rectangle dst_rect = new Rectangle(0, 0, 64, 64);
                            BitmapData bd_src = tmp_bmp.LockBits(src_rect, ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
                            BitmapData bd_dst = tinted.LockBits(dst_rect, ImageLockMode.WriteOnly, PixelFormat.Format32bppArgb);
                            int j, k;
                            IntPtr src_ptr = bd_src.Scan0;
                            IntPtr dst_ptr = bd_dst.Scan0;
                            int src_bytes = Math.Abs(bd_src.Stride) * 64;
                            int dst_bytes = Math.Abs(bd_dst.Stride) * 64;
                            //byte[] src_values = new byte[src_bytes];
                            //byte[] dst_values = new byte[dst_bytes];
                            //System.Runtime.InteropServices.Marshal.Copy(src_ptr, src_values, 0, src_bytes);
                            //System.Runtime.InteropServices.Marshal.Copy(dst_ptr, dst_values, 0, dst_bytes);
                            byte* src_values = (byte*)src_ptr;
                            byte* dst_values = (byte*)dst_ptr;
                            for (k = 0; k < 64; k++)
                            {
                                for (j = 0; j < 64; j++)
                                {
                                    int blue = src_values[k * bd_src.Stride + j * 4 + 0];
                                    int green = src_values[k * bd_src.Stride + j * 4 + 1];
                                    int red = src_values[k * bd_src.Stride + j * 4 + 2];
                                    int alpha = src_values[k * bd_src.Stride + j * 4 + 3];
                                    int value;
                                    if (alpha == 0)
                                    {
                                        value = 0;
                                    }
                                    else
                                    {
                                        int bright = (red + blue + green) / 3;
                                        value = (0xFF << 24) | (bright << 16) | bright;
                                    }
                                    dst_values[k * bd_dst.Stride + j * 4 + 0] = (byte)(value & 0xff);
                                    dst_values[k * bd_dst.Stride + j * 4 + 1] = (byte)((value >> 8) & 0xff);
                                    dst_values[k * bd_dst.Stride + j * 4 + 2] = (byte)((value >> 16) & 0xff);
                                    dst_values[k * bd_dst.Stride + j * 4 + 3] = (byte)((value >> 24) & 0xff);
                                }
                            }
                            //System.Runtime.InteropServices.Marshal.Copy(dst_values, 0, dst_ptr, dst_bytes);
                            bd_dst.Scan0 = (IntPtr)dst_values;
                            tmp_bmp.UnlockBits(bd_src);
                            tinted.UnlockBits(bd_dst);
                            g.DrawImage(tinted, new Point[] { new Point(dx, dy), new Point(dx + General.TILE_SIZE, dy), new Point(dx, dy + General.TILE_SIZE) },
                               dst_rect, GraphicsUnit.Pixel, new ImageAttributes(), null
                           );
                        }
					}
				}
			}
		}

        int mx = mouse_x;
        int my = mouse_y;
        get_grid_pos(ref mx, ref my);

        if (!screenshot && show_tile)
        {
            if (!(mx < 0 || my < 0 || mx >= (size.Width * General.TILE_SIZE) || my >= (size.Height * General.TILE_SIZE)))
            {
                int tx = mx / General.TILE_SIZE;
                int ty = my / General.TILE_SIZE;

                Image img = Tile_Panel.tile_sheets[Tile_Panel.curr_sheet];
                int x1 = Tile_Panel.curr_tile % (1024 / General.TILE_SIZE);
                int y1 = Tile_Panel.curr_tile / (1024 / General.TILE_SIZE);
                x1 *= General.TILE_SIZE;
                y1 *= General.TILE_SIZE;
                int x2 = x1 + General.TILE_SIZE;
                int y2 = y1 + General.TILE_SIZE;
                int dx = tx * General.TILE_SIZE;
                int dy = ty * General.TILE_SIZE;
                General.iso_project(ref dx, ref dy);
                dx -= General.TILE_SIZE / 2;
                dy -= General.TILE_SIZE / 2;
                dx -= hoffs;
                dy -= voffs;
                g.DrawImage(img, new Point[] { new Point(dx, dy), new Point(dx + General.TILE_SIZE, dy), new Point(dx, dy + General.TILE_SIZE) },
                    new Rectangle(x1, y1, x2 - x1, y2 - y1), GraphicsUnit.Pixel, new ImageAttributes(), null);
            }
        }

		for (int i = 0; i < layers.Count; i++) {
			Layer l = layers[i];
			for (int y = 0; y < size.Height; y++) {
				for (int x = 0; x < size.Width; x++) {
					int solid_x = x*General.TILE_SIZE;
					int solid_y = y*General.TILE_SIZE;
					Tile t = l.tiles[y][x];
					if (t.solid == true) {
						int xx = solid_x;
						int yy = solid_y;
						General.iso_project(ref xx, ref yy);
						Point p1 = new Point(xx-hoffs, yy-voffs);
						xx = solid_x+General.TILE_SIZE;
						yy = solid_y;
						General.iso_project(ref xx, ref yy);
						Point p2 = new Point(xx-hoffs, yy-voffs);
						xx = solid_x+General.TILE_SIZE;
						yy = solid_y+General.TILE_SIZE;
						General.iso_project(ref xx, ref yy);
						Point p3 = new Point(xx-hoffs, yy-voffs);
						xx = solid_x;
						yy = solid_y+General.TILE_SIZE;
						General.iso_project(ref xx, ref yy);
						Point p4 = new Point(xx-hoffs, yy-voffs);
						Pen p = new Pen(Color.Red);
						g.DrawLine(p, p1, p2);
						g.DrawLine(p, p2, p3);
						g.DrawLine(p, p3, p4);
						g.DrawLine(p, p4, p1);

						g.DrawLine(p, p1, p3);
						g.DrawLine(p, p2, p4);
					}
				}
			}
		}

		if (show_grid) {
			for (int i = 0; i < size.Height+1; i++) {
				int x1, y1, x2, y2;
				x1 = 0;
				y1 = i*General.TILE_SIZE;
				General.iso_project(ref x1, ref y1);
				x2 = size.Width*General.TILE_SIZE;
				y2 = i*General.TILE_SIZE;
				General.iso_project(ref x2, ref y2);
				g.DrawLine(pen, x1-hoffs, y1-voffs, x2-hoffs, y2-voffs);
			}
			for (int i = 0; i < size.Width+1; i++) {
				int x1, y1, x2, y2;
				x1 = i*General.TILE_SIZE;
				y1 = 0;
				General.iso_project(ref x1, ref y1);
				x2 = i*General.TILE_SIZE;
				y2 = size.Height*General.TILE_SIZE;
				General.iso_project(ref x2, ref y2);
				g.DrawLine(pen, x1-hoffs, y1-voffs, x2-hoffs, y2-voffs);
			}
		}
		
		if (show_height) {
			for (int y = 0; y < size.Height; y++) {
				for (int x = 0; x < size.Width; x++) {
					int dx = x*General.TILE_SIZE;
					int dy = y*General.TILE_SIZE;
					General.iso_project(ref dx, ref dy);
					dx -= General.TILE_SIZE/2;
					//dy -= General.TILE_SIZE/2;
					int h = heightmap[y][x];
					int y1 = (h + 7) * General.TILE_SIZE/2;
					int y2 = y1 + General.TILE_SIZE/2;
					dx -= hoffs;
					dy -= voffs;
					g.DrawImage(height_bmp, new Point[] { new Point(dx, dy), new Point(dx+General.TILE_SIZE, dy), new Point(dx, dy+General.TILE_SIZE/2) },
						new Rectangle(0, y1, 64, y2-y1), GraphicsUnit.Pixel, new ImageAttributes(), null
					);
				}
			}
		}

        if (!screenshot && show_outline)
        {
            if (!(mx < 0 || my < 0 || mx >= (size.Width * General.TILE_SIZE) || my >= (size.Height * General.TILE_SIZE)))
            {
                int tx = mx / General.TILE_SIZE;
                int ty = my / General.TILE_SIZE;
                for (int i = ty; i < ty + 2; i++)
                {
                    int x1, y1, x2, y2;
                    x1 = tx * General.TILE_SIZE;
                    y1 = i * General.TILE_SIZE;
                    General.iso_project(ref x1, ref y1);
                    x2 = (tx+1) * General.TILE_SIZE;
                    y2 = i * General.TILE_SIZE;
                    General.iso_project(ref x2, ref y2);
                    g.DrawLine(p_yellow, x1 - hoffs, y1 - voffs, x2 - hoffs, y2 - voffs);
                }
                for (int i = tx; i < tx + 2; i++)
                {
                    int x1, y1, x2, y2;
                    x1 = i * General.TILE_SIZE;
                    y1 = ty * General.TILE_SIZE;
                    General.iso_project(ref x1, ref y1);
                    x2 = i * General.TILE_SIZE;
                    y2 = (ty+1) * General.TILE_SIZE;
                    General.iso_project(ref x2, ref y2);
                    g.DrawLine(p_yellow, x1 - hoffs, y1 - voffs, x2 - hoffs, y2 - voffs);
                }
            }
        }
    }

	protected override void OnMouseDown(MouseEventArgs ea) {
		mouse_down = true;
		first_down = true;

		save_undo();

		// For clone tool
		int n = Tile_Panel.curr_tile;
		int tx = n % (1024 / General.TILE_SIZE);
		int ty = n / (1024 / General.TILE_SIZE);
		clone_down_tile = new Point(tx, ty);
		
		OnMouseClick(ea);
	}
	
	protected override void OnMouseUp(MouseEventArgs ea) {
		mouse_down = false;
	}
	
	private void get_grid_pos(ref int mx, ref int my) {
		int hoffs = HorizontalScroll.Value;
		int voffs = VerticalScroll.Value;

		mx += hoffs;
		my += voffs;
		
		General.reverse_iso_project(ref mx, ref my);
	}

    private static bool is_mouse_move = false;
	
	protected override void OnMouseMove(MouseEventArgs ea) {
		mouse_x = ea.X;
		mouse_y = ea.Y;

		if (mouse_down) {
            is_mouse_move = true;
			OnMouseClick(ea);
            is_mouse_move = false;
		}

        if (show_tile || show_outline)
        {
            Invalidate();
        }
	}

	private void fill(int new_tile, int new_sheet, int layer, int x, int y, int tile_num, int tile_sheet, Stack<Point> stack, bool check_all_layers, bool fill_height, int height) {
		Point[] neighbors_unclipped = new Point[4] {
			new Point(x-1, y),
			new Point(x+1, y),
			new Point(x, y+1),
			new Point(x, y-1)
		};

		List<Point> neighbors = new List<Point>();
		for (int i = 0; i < 4; i++) {
			Point p = neighbors_unclipped[i];
			if (p.X >= 0 && p.Y >= 0 && p.X < layers[0].tiles[0].Count && p.Y < layers[0].tiles.Count) {
				neighbors.Add(p);
			}
		}

		bool[] spread = new bool[neighbors.Count];
	
		if (check_all_layers) {
			for (int i = 0; i < neighbors.Count; i++) {
				Point p = neighbors[i];
				bool go = true;
				for (int l = 0; l < layers.Count; l++) {
					Tile t2 = layers[l].tiles[p.Y][p.X];
					if (l == layer) {
						if (t2.tile_num != tile_num || t2.tile_sheet != tile_sheet) {
							go = false;
							break;
						}
					}
					else {
						if ((t2.tile_num != -1) && (t2.tile_num != tile_num || t2.tile_sheet != tile_sheet)) {
							go = false;
							break;
						}
					}
				}
				if (go) {
					spread[i] = true;
				}
				else {
					spread[i] = false;
				}
			}
		}
		else {
			for (int i = 0; i < neighbors.Count; i++) {
				Point p = neighbors[i];
				Tile t2 = layers[layer].tiles[p.Y][p.X];
				if (t2.tile_num != tile_num || t2.tile_sheet != tile_sheet) {
					spread[i] = false;
				}
				else {
					spread[i] = true;
				}
			}
		}

		for (int i = 0; i < neighbors.Count; i++) {
			if (spread[i] == false) {
				continue;
			}
			Point p = neighbors[i];
			if (fill_height) {
				if (heightmap[p.Y][p.X] != height) {
					stack.Push(p);
				}
			}
			else {
				if (layers[layer].tiles[p.Y][p.X].tile_num != new_tile ||
						 layers[layer].tiles[p.Y][p.X].tile_sheet != new_sheet) {
					stack.Push(p);
				}
			}
		}
	
		if (fill_height) {
			heightmap[y][x] = height;
		}
		else {
			Tile t = layers[layer].tiles[y][x];
			t.tile_num = new_tile;
			t.tile_sheet = new_sheet;
		}
	}
	
	private bool is_aligned(int tx, int ty, Point p) {
		int x, y;

		Point tmp = p;

		while (tmp.Y < size.Height-1) {
			tmp.Y++;
			tmp.X--;
		}

		while (tmp.X+4 < size.Width)
			tmp.X += 4;
	
		x = tmp.X;
		y = size.Height-1;

		for (int i = 0; i < (size.Width+size.Height)/4; i++) {
			int xx, yy;
			xx = x - (i * 4);
			yy = y;
			for (; yy >= 0; xx++, yy--) {
				if (xx < 0 || xx >= size.Width) {
					continue;
				}
				if (tx == xx && ty == yy) {
					return true;
				}
			}
		}

		return false;
	}

	private void get_screen_pos(ref int tx, ref int ty) {
		tx *= General.TILE_SIZE;
		ty *= General.TILE_SIZE;
		General.iso_project(ref tx, ref ty);
		tx += 1;
		ty += 1;
	}

    private class C : IComparer<Point>
    {
        public int Compare(Point x, Point y)
        {
            if (x.X == y.X && x.Y == y.Y)
                return 0;
            else if (x.Y == y.Y && x.X > y.X)
                return 1;
            else if (x.Y > y.Y)
                return 1;
            else
                return -1;
        }
    }

	protected override void OnMouseClick(MouseEventArgs ea) {
		int mx = ea.X;
		int my = ea.Y;

		get_grid_pos(ref mx, ref my);

		if (mx < 0 || my < 0 || mx >= (size.Width*General.TILE_SIZE) || my >= (size.Height*General.TILE_SIZE))
			return;

		int tx = mx/General.TILE_SIZE;
		int ty = my/General.TILE_SIZE;
		Tile t = layers[layer].tiles[ty][tx];

		if (Tool_Window.tools[Tool_Window.curr_tool] == Tool_Window.Tool_Type.PAINT) {
			t.tile_sheet = Tile_Panel.curr_sheet;
			t.tile_num = Tile_Panel.curr_tile;
		}
        else if (Tool_Window.tools[Tool_Window.curr_tool] == Tool_Window.Tool_Type.INFO && !is_mouse_move)
        {
            String s = String.Format("Grid x/y: {0},{1}", tx, ty);
            MessageBox.Show(s);
        }
        else if (Tool_Window.tools[Tool_Window.curr_tool] == Tool_Window.Tool_Type.LAYER_PAINT)
        {
            Point p = new Point(tx, ty);
            if (src_layer < layers.Count &&
                layer_painted.BinarySearch(p, new C()) < 0 &&
                layers[src_layer].tiles[ty][tx].tile_num >= 0)
            {
                t.tile_num = layers[src_layer].tiles[ty][tx].tile_num;
                t.tile_sheet = layers[src_layer].tiles[ty][tx].tile_sheet;
                layers[src_layer].tiles[ty][tx].tile_sheet = 0;
                layers[src_layer].tiles[ty][tx].tile_num = -1;
                layer_painted.Add(p);
                layer_painted.Sort(new C());
            }
        }
        else if (Tool_Window.tools[Tool_Window.curr_tool] == Tool_Window.Tool_Type.CLONE)
        {
            if (first_down)
            {
                clone_down_iso = new Point(tx, ty);
                clone_prev_iso = clone_down_iso;
                int my_mx = tx;
                int my_my = ty;
                get_screen_pos(ref my_mx, ref my_my);
                clone_down_screen = new Point(my_mx, my_my);
            }
            Point this_iso = new Point(tx, ty);
            int diff_x = this_iso.X - clone_prev_iso.X;
            int diff_y = this_iso.Y - clone_prev_iso.Y;
            int dx = 0;
            int dy = 0;
            int found_i = -1;
            if (diff_x != 0 || diff_y != 0)
            {
                for (int i = 0; i < 4; i++)
                {
                    if (
                        (Math.Sign(diff_x) == Math.Sign(directions[i].X) && Math.Abs(diff_x) >= Math.Abs(directions[i].X)) &&
                        (Math.Sign(diff_y) == Math.Sign(directions[i].Y) && Math.Abs(diff_y) >= Math.Abs(directions[i].Y))
                    )
                    {
                        found_i = i;
                        break;
                    }
                }
                if (found_i >= 0)
                {
                    dx = directions[found_i].X;
                    dy = directions[found_i].Y;
                }
            }
            if (first_down || (found_i >= 0 && is_aligned(clone_prev_iso.X + dx, clone_prev_iso.Y + dy, this_iso) && this_iso != clone_prev_iso))
            {
                first_down = false;
                clone_prev_iso.X += dx;
                clone_prev_iso.Y += dy;
                //iso_to_tilemap(ref dx, ref dy);
                int my_mx = tx, my_my = ty;
                get_screen_pos(ref my_mx, ref my_my);
                int new_tx = clone_down_tile.X + (my_mx - clone_down_screen.X) / General.TILE_SIZE;
                int new_ty = clone_down_tile.Y + (my_my - clone_down_screen.Y) / General.TILE_SIZE;
                int num = new_tx + (new_ty * (1024 / General.TILE_SIZE));
                Tile_Panel.curr_tile = num;
                t.tile_sheet = Tile_Panel.curr_sheet;
                t.tile_num = Tile_Panel.curr_tile;
            }
            tw.panel.Invalidate();
        }
        else if (Tool_Window.tools[Tool_Window.curr_tool] == Tool_Window.Tool_Type.CLEAR)
        {
            t.tile_sheet = 0;
            t.tile_num = -1;
        }
        else if (Tool_Window.tools[Tool_Window.curr_tool] == Tool_Window.Tool_Type.SOLID)
        {
            t.solid = !t.solid;
        }
        else if (Tool_Window.tools[Tool_Window.curr_tool] == Tool_Window.Tool_Type.FILL)
        {
            bool fill_all = Tool_Window.fill_all.Checked;
            int tile_num = t.tile_num;
            int tile_sheet = t.tile_sheet;

            Stack<Point> stack = new Stack<Point>();

            stack.Push(new Point(tx, ty));

            while (stack.Count > 0)
            {
                Point p = stack.Pop();

                fill(Tile_Panel.curr_tile, Tile_Panel.curr_sheet, layer, p.X, p.Y, tile_num, tile_sheet, stack, fill_all, false, 0);
            }
        }
        else if (Tool_Window.tools[Tool_Window.curr_tool] == Tool_Window.Tool_Type.HEIGHT_FILL)
        {
            int tile_num = t.tile_num;
            int tile_sheet = t.tile_sheet;

            Stack<Point> stack = new Stack<Point>();

            stack.Push(new Point(tx, ty));

            while (stack.Count > 0)
            {
                Point p = stack.Pop();

                fill(Tile_Panel.curr_tile, Tile_Panel.curr_sheet, layer, p.X, p.Y, tile_num, tile_sheet, stack, false, true, Tool_Window.curr_height);
            }
        }
        else if (Tool_Window.tools[Tool_Window.curr_tool] == Tool_Window.Tool_Type.HEIGHT_PAINT)
        {
            heightmap[ty][tx] = Tool_Window.curr_height;
        }

		Invalidate();
	}
	
	static readonly object _locker = new object();
	public void handle_key(Key_Info ki) {
		lock (_locker) {
			int hoffs = HorizontalScroll.Value;
			int voffs = VerticalScroll.Value;

			int mx = mouse_x;
			int my = mouse_y;

			mx += hoffs;
			my += voffs;
			
			General.reverse_iso_project(ref mx, ref my);

			// keep this up to date!
			if (ki.KeyCode != Keys.L && ki.KeyCode != Keys.X) {
				if (mx < 0 || my < 0 || mx >= (size.Width*General.TILE_SIZE) || my >= (size.Height*General.TILE_SIZE)) {
					return;
				}
			}

			mx /= General.TILE_SIZE;
			my /= General.TILE_SIZE;

			save_undo();

			if (ki.KeyCode == Keys.R && ki.Control && ki.Shift) {
				List<Tile> row = new List<Tile>();
				List<int> hrow = new List<int>();
				for (int i = 0; i < size.Width; i++) {
					Tile t = new Tile();
					row.Add(t);
					hrow.Add(0);
				}
				for (int i = 0; i < layers.Count; i++) {
					layers[i].tiles.Insert(my+1, row);
				}
				heightmap.Insert(my+1, hrow);
				size.Height++;
				resize();
			}
			else if (ki.KeyCode == Keys.R && ki.Control) {
				List<Tile> row = new List<Tile>();
				List<int> hrow = new List<int>();
				for (int i = 0; i < size.Width; i++) {
					Tile t = new Tile();
					row.Add(t);
					hrow.Add(0);
				}
				for (int i = 0; i < layers.Count; i++) {
					layers[i].tiles.Insert(my, row);
				}
				heightmap.Insert(my, hrow);
				size.Height++;
				resize();
			}
			else if (ki.KeyCode == Keys.C && ki.Control && ki.Shift) {
				for (int i = 0; i < layers.Count; i++) {
					for (int y = 0; y < size.Height; y++) {
						Tile t = new Tile();
						layers[i].tiles[y].Insert(mx+1, t);
						if (i == 0) {
							heightmap[y].Insert(mx+1, 0);
						}
					}
				}
				size.Width++;
				resize();
			}
			else if (ki.KeyCode == Keys.C && ki.Control) {
				for (int i = 0; i < layers.Count; i++) {
					for (int y = 0; y < size.Height; y++) {
						Tile t = new Tile();
						layers[i].tiles[y].Insert(mx, t);
						if (i == 0) {
							heightmap[y].Insert(mx, 0);
						}
					}
				}
				size.Width++;
				resize();
			}
			else if (ki.KeyCode == Keys.D && ki.Control && ki.Shift) {
				for (int i = 0; i < layers.Count; i++) {
					for (int y = 0; y < size.Height; y++) {
						layers[i].tiles[y].RemoveAt(mx);
						if (i == 0) {
							heightmap[y].RemoveAt(mx);
						}
					}
				}
				size.Width--;
				resize();
			}
			else if (ki.KeyCode == Keys.D && ki.Control) {
				for (int i = 0; i < layers.Count; i++) {
					layers[i].tiles.RemoveAt(my);
					if (i == 0) {
						heightmap.RemoveAt(my);
					}
				}
				size.Height--;
				resize();
			}
			else if (ki.KeyCode == Keys.L && ki.Control && ki.Shift) {
				Layer layer0 = new Layer();
				layer0.tiles = new List<List<Tile>>();
				for (int i = 0; i < size.Height; i++) {
					List<Tile> l = new List<Tile>();
					for (int j = 0; j < size.Width; j++) {
						Tile t = new Tile();
						l.Add(t);
					}
					layer0.tiles.Add(l);
				}
				layers.Insert(layer+1, layer0);
				layer++;
			}
			else if (ki.KeyCode == Keys.L && ki.Control) {
				Layer layer0 = new Layer();
				layer0.tiles = new List<List<Tile>>();
				for (int i = 0; i < size.Height; i++) {
					List<Tile> l = new List<Tile>();
					for (int j = 0; j < size.Width; j++) {
						Tile t = new Tile();
						l.Add(t);
					}
					layer0.tiles.Add(l);
				}
				layers.Insert(layer, layer0);
			}
			else if (ki.KeyCode == Keys.X && ki.Control) {
				if (layers.Count > 0) {
					layers.RemoveAt(layer);
                    if (layer >= layers.Count)
                    {
                        layer--;
                        Tool_Window.layer_label.Text = "Layer " + layer;
                        pw.recreate_layers_menu();
                    }
				}
			}
			else {
				return;
			}

			Invalidate();
		}
	}

	public void undo() {
        int old_layer = layer;

		if (undo_stack.Count <= 0) {
			return;
		}
		if (redo_stack.Count >= MAX_UNDO) {
			redo_stack.RemoveAt(MAX_UNDO-1);
			heightmap_redo_stack.RemoveAt(MAX_UNDO-1);
		}
		List<Layer> layer_clone;
		List<List<int>> heightmap_clone;
		clone_map(layers, heightmap, out layer_clone, out heightmap_clone);
		redo_stack.Insert(0, layer_clone);
		heightmap_redo_stack.Insert(0, heightmap_clone);

		clone_map(undo_stack[0], heightmap_undo_stack[0], out layers, out heightmap);
		undo_stack.RemoveAt(0);
		heightmap_undo_stack.RemoveAt(0);

        if (old_layer >= layers.Count)
        {
            layer = 0;
            Tool_Window.layer_label.Text = "Layer 0";
        }
        pw.recreate_layers_menu();

        System.Console.WriteLine("{0} {1} {2}", layer, old_layer, layers.Count);
		size = new Size(layers[0].tiles[0].Count, layers[0].tiles.Count);
		resize();

		Invalidate();
	}

	public void redo() {
        int old_layer = layer;

		if (redo_stack.Count <= 0) {
			return;
		}
		if (undo_stack.Count >= MAX_UNDO) {
			undo_stack.RemoveAt(MAX_UNDO-1);
			heightmap_undo_stack.RemoveAt(MAX_UNDO-1);
		}
		List<Layer> layer_clone;
		List<List<int>> heightmap_clone;
		clone_map(layers, heightmap, out layer_clone, out heightmap_clone);

		undo_stack.Insert(0, layer_clone);
		heightmap_undo_stack.Insert(0, heightmap_clone);

		clone_map(redo_stack[0], heightmap_redo_stack[0], out layers, out heightmap);
		redo_stack.RemoveAt(0);
		heightmap_redo_stack.RemoveAt(0);

        if (old_layer >= layers.Count)
        {
            layer = 0;
            Tool_Window.layer_label.Text = "Layer 0";
        }
        pw.recreate_layers_menu();

        layer = 0;
		size = new Size(layers[0].tiles[0].Count, layers[0].tiles.Count);
		resize();

		Invalidate();
	}

	private void write_byte(FileStream fs, byte b) {
		fs.WriteByte(b);
	}

	private void write_int32(FileStream fs, int i) {
		write_byte(fs, (byte)(i & 0xff));
		write_byte(fs, (byte)((i >> 8) & 0xff));
		write_byte(fs, (byte)((i >> 16) & 0xff));
		write_byte(fs, (byte)((i >> 24) & 0xff));
	}

	private void really_save_heightmap(string filename) {
		FileStream output = new FileStream(filename, FileMode.Create, FileAccess.Write);

		for (int y = 0; y < heightmap.Count; y++) {
			for (int x = 0; x < heightmap[0].Count; x += 2) {
				int h1 = heightmap[y][x];
				int h2;
				if (x+1 < heightmap[0].Count)
					h2 = heightmap[y][x+1];
				else
					h2 = 0;
				h1 += 7;
				h2 += 7;
				write_byte(output, (byte)(h1 | (h2 << 4)));
			}
		}

		output.Close();
	}

	public void save_heightmap() {
		SaveFileDialog sfd = new SaveFileDialog();
		sfd.DefaultExt = "";
		sfd.OverwritePrompt = true;
		DialogResult dr = sfd.ShowDialog();
		if (dr == DialogResult.OK) {
			really_save_heightmap(sfd.FileName);
		}
	}

	private void really_save(string filename) {
		FileStream output = new FileStream(filename, FileMode.Create, FileAccess.Write);

		write_int32(output, layers[0].tiles[0].Count);
		write_int32(output, layers[0].tiles.Count);
		write_int32(output, layers.Count);

		for (int l = 0; l < layers.Count; l++) {
			for (int y = 0; y < layers[l].tiles.Count; y++) {
				for (int x = 0; x < layers[l].tiles[0].Count; x++) {
					Tile t = layers[l].tiles[y][x];
					write_int32(output, t.tile_num);
					write_byte(output, (byte)t.tile_sheet);
					write_byte(output, (byte)(t.solid == true ? 1 : 0));
				}
			}
		}

		output.Close();
	}

	public void save() {
		SaveFileDialog sfd = new SaveFileDialog();
		sfd.DefaultExt = "";
		sfd.OverwritePrompt = true;
		DialogResult dr = sfd.ShowDialog();
		if (dr == DialogResult.OK) {
			really_save(sfd.FileName);
		}
	}

	int read_byte(FileStream fs) {
		return fs.ReadByte();
	}

	int read_int32(FileStream fs) {
		byte[] b = new byte[4];

		for (int i = 0; i < 4; i++) {
			b[i] = (byte)read_byte(fs);
		}

		return (b[0]) | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
	}

	private void load_heightmap(string filename) {
		FileStream input = new FileStream(filename, FileMode.Open, FileAccess.Read);
	
		int w = size.Width;
		int h = size.Height;

		List<List<int>> backup = heightmap;

		heightmap = new List<List<int>>();
		for (int y = 0; y < h; y++) {
			List<int> row = new List<int>();
			for (int x = 0; x < w; x += 2) {
				int two = read_byte(input);
				if (two == -1) {
					MessageBox.Show("An error occurred. The size of the heightmap may not match the size of the level. Restoring previous heightmap.");
					heightmap = backup;
					return;
				}
				int h1 = (two & 0xF);
				int h2 = (two >> 4) & 0xF;
				h1 -= 7;
				h2 -= 7;
				row.Add(h1);
				if ((x+1) < w)
					row.Add(h2);
			}
			heightmap.Add(row);
		}

		input.Close();
		
		Invalidate();
	}

	public void load_heightmap() {
		OpenFileDialog ofd = new OpenFileDialog();
		ofd.DefaultExt = "";
		DialogResult dr = ofd.ShowDialog();
		if (dr == DialogResult.OK) {
			load_heightmap(ofd.FileName);
			heightmap_undo_stack.Clear();
			heightmap_redo_stack.Clear();
		}
	}

	private void load(string filename) {
		layers.Clear();

		FileStream input = new FileStream(filename, FileMode.Open, FileAccess.Read);

		int w = read_int32(input);
		int h = read_int32(input);
		int nl = read_int32(input);

		size = new Size(w, h);

		layer = 0;
        Tool_Window.layer_label.Text = "Layer 0";
		resize();

		for (int i = 0; i < nl; i++) {
			Layer l = new Layer();
			l.tiles = new List<List<Tile>>();
			for (int y = 0; y < h; y++) {
				List<Tile> row = new List<Tile>();
				for (int x = 0; x < w; x++) {
					Tile t = new Tile();
					t.tile_num = read_int32(input);
					t.tile_sheet = read_byte(input);
					t.solid = read_byte(input) == 1 ? true : false;
					row.Add(t);
				}
				l.tiles.Add(row);
			}
			layers.Add(l);
		}

		input.Close();

		init_heightmap(size.Width, size.Height);

		pw.recreate_layers_menu();

		Invalidate();
	}

	public void load() {
		OpenFileDialog ofd = new OpenFileDialog();
		ofd.DefaultExt = "";
		DialogResult dr = ofd.ShowDialog();
		if (dr == DialogResult.OK) {
			load(ofd.FileName);
			undo_stack.Clear();
			redo_stack.Clear();
			top = new Point(0, 0);
		}
	}

	/*
	[DllImport("winmm.dll")]
	static extern int timeGetTime();	

	private int next_invalidate = 0;
	private bool should_invalidate = false;

	private void My_Invalidate() {
		if (timeGetTime() > next_invalidate) {
			next_invalidate = timeGetTime() + 10;
			should_invalidate = true;
		}
	}

	void invalidator() {
		while (true) {
			Thread.Sleep(10);
			if (timeGetTime() > next_invalidate && should_invalidate) {
				should_invalidate = false;
				handle_keys();
			}
		}
	}
	*/
}

class Main_Window : Form {
	public Main_Panel panel;

	public Main_Window(Parent_Window pw, Tile_Window tw) {
		panel = new Main_Panel(pw, tw);
		panel.Parent = this;
		panel.Location = new Point(0, 0);
	}

	protected override void OnFormClosing(FormClosingEventArgs e) {
		if (e.CloseReason == CloseReason.MdiFormClosing) {
			return;
		}
		Visible = false;
		e.Cancel = true;
	}
}
