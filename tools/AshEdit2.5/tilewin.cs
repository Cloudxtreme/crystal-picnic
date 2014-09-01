using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Windows.Forms;
using System.Collections.Generic;
using System.IO;

class Tile_Panel : Panel {
	public static List<Bitmap> tile_sheets = new List<Bitmap>();

	public static int curr_sheet = 0;
	public static int curr_tile = 0;

	public Tile_Panel() {
		AutoScroll = true;
		Dock = DockStyle.Fill;

		try {
			for (int i = 0; i < 256; i++) {
				Bitmap tmp = new Bitmap(String.Format("tiles{0}.png", i));
				Bitmap bmp = new Bitmap(tmp.Width, tmp.Height, PixelFormat.Format32bppPArgb);
				Graphics g = Graphics.FromImage(bmp);
				g.InterpolationMode = InterpolationMode.High;
				g.DrawImage(tmp, new Rectangle(0, 0, bmp.Width, bmp.Height));
				g.Dispose();
				tile_sheets.Add(bmp);
			}
		}
		catch (ArgumentException) {
		}
		
		resize();
		
		DoubleBuffered = true;
	}

	public void resize() {
		AutoScrollMinSize = new Size(1024, 1024);;
	}

	protected override void OnPaint(PaintEventArgs pea) {
		Graphics g = pea.Graphics;

		int hoffs = HorizontalScroll.Value;
		int voffs = VerticalScroll.Value;

		g.DrawImage(tile_sheets[curr_sheet], new Point(-hoffs, -voffs));

		int tx = curr_tile % (1024/General.TILE_SIZE);
		int ty = curr_tile / (1024/General.TILE_SIZE);

		Pen pen = new Pen(Color.Yellow);
		g.DrawRectangle(pen, tx*General.TILE_SIZE-hoffs, ty*General.TILE_SIZE-voffs, General.TILE_SIZE, General.TILE_SIZE);
		g.DrawRectangle(pen, tx*General.TILE_SIZE+1-hoffs, ty*General.TILE_SIZE+1-voffs, General.TILE_SIZE-2, General.TILE_SIZE-2);
	}

	protected override void OnMouseClick(MouseEventArgs ea) {
		int hoffs = HorizontalScroll.Value;
		int voffs = VerticalScroll.Value;
		curr_tile = ((ea.X+hoffs)/General.TILE_SIZE) + ((ea.Y+voffs)/General.TILE_SIZE)*(1024/General.TILE_SIZE);
		Invalidate();
	}
}

class Tile_Window : Form {
	public Tile_Panel panel;

	private Parent_Window pw;

	public Tile_Window(Parent_Window pw) {
		this.pw = pw;
		panel = new Tile_Panel();
		panel.Parent = this;
		panel.Location = new Point(0, 0);;
	}

	protected override void OnFormClosing(FormClosingEventArgs e) {
		if (e.CloseReason == CloseReason.MdiFormClosing) {
			return;
		}
		Visible = false;
		e.Cancel = true;
	}
}
