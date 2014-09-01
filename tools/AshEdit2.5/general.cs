using System;
using System.Drawing;

class General {
	public const int TILE_SIZE = 64;
	public static Point iso_offset;

	public static void iso_project(ref int x, ref int y) {
		double xinc = -y / 2.0;
		double yinc = x / 4.0;
		
		x = (int)(((float)x/2) + xinc);
		y = (int)(((float)y/4) + yinc);

		x += iso_offset.X;
		y += iso_offset.Y;
	}

	public static void reverse_iso_project(ref int x, ref int y) {
		x -= iso_offset.X;
		y -= iso_offset.Y;

		double xinc = -2*y;
		double yinc = x / 2;
	
		x = (int)(x - xinc);
		y = (int)(2 * (y - yinc));
	}
}
