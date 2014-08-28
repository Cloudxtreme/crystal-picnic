using System;
using System.Drawing;
using System.Windows.Forms;
using System.IO;
using System.Collections.Generic;
using System.Text;

using de.mathertel;

class TextLists {
	public List<string> english_lines;
	public List<string> old_english_lines;
	public List<string> translated_lines;

	public TextLists() {
		english_lines = new List<string>();
		old_english_lines = new List<string>();
		translated_lines = new List<string>();
	}

	public void read_english() {
		read_file("english.dat", english_lines);
	}

	public void read_old_english() {
		read_file("english.dat.old", old_english_lines);
	}

	public void read_translation(string filename) {
		read_file(filename, translated_lines);
	}

	public void save_translation(string filename) {
		StreamWriter output_writer = null;

		try {
			FileStream output = new FileStream(filename, FileMode.Create, FileAccess.Write);
			output_writer = new StreamWriter(output, new UTF8Encoding());

			foreach (string line in translated_lines) {
				output_writer.WriteLine(line);
			}
		}
		catch (Exception e) {
		}
		finally {
			if (output_writer != null)
				output_writer.Close();
		}
	}

	private void read_file(string filename, List<string> lines) {
		lines.Clear();

		StreamReader input_reader = null;

		try {
			FileStream input = new FileStream(filename, FileMode.Open, FileAccess.Read);
			input_reader = new StreamReader(input, new UTF8Encoding());

			string line;
			while ((line = input_reader.ReadLine()) != null) {
				lines.Add(line);
			}

			input_reader.Close();
		}
		catch (Exception e) {
		}
		finally {
			if (input_reader != null) {
				input_reader.Close();
			}
		}
	}
}

class DiffForm : Form {
	public DiffForm(RichTextBox b) {
		Text = "Difference";
		FormBorderStyle = FormBorderStyle.FixedSingle;
		ClientSize = new Size(600, 500);
		
		b.Parent = this;
		b.Location = new Point(5, 5);
		b.Size = new Size(590, 490);
		b.ReadOnly = true;
		b.Multiline = true;

		CenterToScreen();
	}
}

class EditForm : Form {
	private TextBox english_box;
	private TextBox translate_box;
	private Label number_label;
	private Button next_button;
	private Button prev_button;
	private Button compare_button;
	private Button store_button;
	private Button reset_button;
	private TextLists lists;
	private int current = 0;

	public EditForm(TextLists lists) {
		Text = "Edit translation";

		this.lists = lists;

		FormBorderStyle = FormBorderStyle.FixedSingle;

		ClientSize = new Size(600, 500);

		english_box = new TextBox();
		english_box.Parent = this;
		english_box.Location = new Point(5, 5);
		english_box.Size = new Size(500, 240);
		english_box.ReadOnly = true;
		english_box.Multiline = true;

		translate_box = new TextBox();
		translate_box.Parent = this;
		translate_box.Location = new Point(5, 255);
		translate_box.Size = new Size(500, 240);
		translate_box.Multiline = true;
		translate_box.KeyPress += new KeyPressEventHandler(key_pressed);

		number_label = new Label();
		number_label.Parent = this;
		number_label.Location = new Point(510, 5);
		next_button = new Button();
		next_button.Text = "Next";
		next_button.Parent = this;
		next_button.Location = new Point(510, 40);
		next_button.Click += new EventHandler(next_button_OnClick);
		prev_button = new Button();
		prev_button.Text = "Previous";
		prev_button.Parent = this;
		prev_button.Location = new Point(510, 60);
		prev_button.Click += new EventHandler(prev_button_OnClick);
		compare_button = new Button();
		compare_button.Text = "Compare";
		compare_button.Parent = this;
		compare_button.Location = new Point(510, 95);
		compare_button.Click += new EventHandler(compare_button_OnClick);

		store_button = new Button();
		store_button.Text = "Store";
		store_button.Parent = this;
		store_button.Location = new Point(510, 400);
		store_button.Click += new EventHandler(store_button_OnClick);
		reset_button = new Button();
		reset_button.Text = "Reset";
		reset_button.Parent = this;
		reset_button.Location = new Point(510, 420);
		reset_button.Click += new EventHandler(reset_button_OnClick);

		CenterToScreen();
	}

	private void set_label() {
		number_label.Text = "" + (current+1) + " of " + lists.english_lines.Count;
	}

	private void set_text() {
		english_box.Text = lists.english_lines[current];
		translate_box.Text = lists.translated_lines[current];
	}

	private void key_pressed(Object o, KeyPressEventArgs e) {
		if (e.KeyChar == (char)Keys.Return) {
			e.Handled = true;
		}
	}
	
	protected override void OnFormClosed(FormClosedEventArgs e) {
		maybe_store();
	}

	public void ShowIt() {
		// Make all of the lists the same size
		int diff = lists.old_english_lines.Count - lists.english_lines.Count;
		if (diff < 0) {
			for (int i = diff; i < 0; i++) {
				lists.old_english_lines.Add("");
			}
		}
		else if (diff > 0) {
			lists.old_english_lines.RemoveRange(lists.english_lines.Count+1, diff);
		}
		diff = lists.translated_lines.Count - lists.english_lines.Count;
		if (diff < 0) {
			for (int i = diff; i < 0; i++) {
				lists.translated_lines.Add("");
			}
		}
		else if (diff > 0) {
			lists.translated_lines.RemoveRange(lists.english_lines.Count+1, diff);
		}

		set_text();
		set_label();

		ShowDialog();
	}

	private void store() {
		lists.translated_lines[current] = translate_box.Text;
	}

	private void reset() {
		translate_box.Text = lists.translated_lines[current];
	}

	private void maybe_store() {
		DialogResult dr = MessageBox.Show("Store?", "Alert", MessageBoxButtons.YesNo);
		if (dr == DialogResult.Yes) {
			store();
		}
	}

	public void next_button_OnClick(object sender, EventArgs ea) {
		if (current < lists.english_lines.Count-1) {
			maybe_store();
			current++;
			set_text();
			set_label();
		}
	}

	public void prev_button_OnClick(object sender, EventArgs ea) {
		if (current > 0) {
			maybe_store();
			current--;
			set_text();
			set_label();
		}
	}

	public void store_button_OnClick(object sender, EventArgs ea) {
		store();
	}

	public void reset_button_OnClick(object sender, EventArgs ea) {
		reset();
	}

	struct SelectionChunk {
		public int start;
		public int len;
		public Color color;
	}

	public void compare_button_OnClick(object sender, EventArgs ea) {
		string a_line = lists.old_english_lines[current];
		string b_line = lists.english_lines[current];

		int[] a_codes = Diff.DiffCharCodes(a_line, false);
		int[] b_codes = Diff.DiffCharCodes(b_line, false);
		Diff.Item[] diffs = Diff.DiffInt(a_codes, b_codes);

		RichTextBox richtext_box = new RichTextBox();
		richtext_box.Text = "";
		Color orig = richtext_box.SelectionBackColor;
		List<SelectionChunk> l = new List<SelectionChunk>();
		string text = "";
		int start, len;
		SelectionChunk se;

		int pos = 0;
		for (int n = 0; n < diffs.Length; n++) {
			Diff.Item it = diffs[n];
		
			start = text.Length;
			len = 0;

			// write unchanged chars
			while ((pos < it.StartB) && (pos < b_line.Length)) {
				text += b_line[pos];
				pos++;
				len++;
			} // while

			se = new SelectionChunk();
			se.start = start;
			se.len = len;
			se.color = orig;
			l.Add(se);

			start = text.Length;
			len = 0;

			// write deleted chars
			if (it.deletedA > 0) {
				for (int m = 0; m < it.deletedA; m++) {
					text += a_line[it.StartA + m];
					len++;
				} // for
			}
			
			se = new SelectionChunk();
			se.start = start;
			se.len = len;
			se.color = Color.Pink;
			l.Add(se);

			start = text.Length;
			len = 0;

			// write inserted chars
			if (pos < it.StartB + it.insertedB) {
				while (pos < it.StartB + it.insertedB) {
					text += b_line[pos];
					pos++;
					len++;
				} // while
			} // if

			se = new SelectionChunk();
			se.start = start;
			se.len = len;
			se.color = Color.LightGreen;
			l.Add(se);
		} // while

		start = text.Length;
		len = 0;
	
		// write rest of unchanged chars
		while (pos < b_line.Length) {
			text += b_line[pos];
			pos++;
			len++;
		} // while

		se = new SelectionChunk();
		se.start = start;
		se.len = len;
		se.color = orig;
		l.Add(se);

		richtext_box.Text = text;

		for (int i = 0; i < l.Count; i++) {
			richtext_box.Select(l[i].start, l[i].len);
			richtext_box.SelectionBackColor = l[i].color;
		}

		richtext_box.DeselectAll();

		DiffForm df = new DiffForm(richtext_box);
		df.Visible = true;
	}
}

class MainForm : Form {
	private MainMenu menu;
	private MenuItem action_menu;
	private MenuItem action_import_english = new MenuItem();
	private MenuItem action_load_translation = new MenuItem();
	private MenuItem action_save_translation = new MenuItem();
	private MenuItem action_exit = new MenuItem();
	private Label loaded_translation_name;
	private Button edit_button;
	private TextLists lists;
	private EditForm edit_form;

	public MainForm(TextLists lists) {
		Text = "Translate";

		this.lists = lists;

		FormBorderStyle = FormBorderStyle.FixedSingle;

		action_import_english.Text = "Import new English data";
		action_import_english.Click += new EventHandler(import_english_OnClick);
		action_load_translation.Text = "Load translation";
		action_load_translation.Click += new EventHandler(load_translation_OnClick);
		action_save_translation.Text = "Save translation";
		action_save_translation.Click += new EventHandler(save_translation_OnClick);
		action_exit.Text = "Exit";
		action_exit.Click += new EventHandler(exit_OnClick);
		action_menu = new MenuItem("Actions", new MenuItem[] {
			action_import_english,
			action_load_translation,
			action_save_translation,
			action_exit
		});
		menu = new MainMenu(new MenuItem[] { action_menu });
		Menu = menu;

		loaded_translation_name = new Label();
		loaded_translation_name.Text = "No translation loaded.";
		loaded_translation_name.Parent = this;
		loaded_translation_name.Location = new Point(5, 5);
		loaded_translation_name.AutoSize = true;
		loaded_translation_name.Width = loaded_translation_name.PreferredWidth;

		edit_button = new Button();
		edit_button.Text = "Start Editing";
		edit_button.Parent = this;
		edit_button.Location = new Point(5, 45);
		edit_button.Click += new EventHandler(edit_button_OnClick);
		edit_button.BackColor = SystemColors.ButtonFace;

		ClientSize = new Size(300, 200);

		CenterToScreen();

		BackColor = Color.White;
	}

	public void import_english_OnClick(object sender, EventArgs ea) {
		OpenFileDialog ofd = new OpenFileDialog();
		ofd.DefaultExt = "dat";
		DialogResult dr = ofd.ShowDialog();
		if (dr == DialogResult.OK) {
			dr = MessageBox.Show("Really import this?", "Import New English data", MessageBoxButtons.YesNo);
			if (dr == DialogResult.Yes) {
				System.IO.File.Copy("english.dat", "english.dat.old", true);
				System.IO.File.Copy(ofd.FileName, "english.dat", true);
				lists.read_english();
				lists.read_old_english();
			}
		}
	}

	public void load_translation_OnClick(object sender, EventArgs ea) {
		OpenFileDialog ofd = new OpenFileDialog();
		ofd.DefaultExt = "dat";
		DialogResult dr = ofd.ShowDialog();
		if (dr == DialogResult.OK) {
			dr = MessageBox.Show("Really import this?", "Import Translation", MessageBoxButtons.YesNo);
			if (dr == DialogResult.Yes) {
				lists.read_translation(ofd.FileName);
				loaded_translation_name.Text = ofd.FileName;
				int pw =  loaded_translation_name.PreferredWidth;
				ClientSize = new Size(pw + 10, ClientSize.Height);
			}
		}
	}

	public void save_translation_OnClick(object sender, EventArgs ea) {
		SaveFileDialog sfd = new SaveFileDialog();
		sfd.DefaultExt = "dat";
		DialogResult dr = sfd.ShowDialog();
		if (dr == DialogResult.OK) {
			lists.save_translation(sfd.FileName);
		}
	}

	public void exit_OnClick(object sender, EventArgs ea) {
		Close();
	}

	public void edit_button_OnClick(object sender, EventArgs ea) {
		edit_form = new EditForm(lists);
		edit_form.ShowIt();
	}
	
	protected override void OnFormClosing(FormClosingEventArgs e) {
		DialogResult dr = MessageBox.Show("Really exit?", "Attention", MessageBoxButtons.YesNo);
		if (dr == DialogResult.Yes) {
			return;
		}
		e.Cancel = true;
	}
}

class EntryPoint {
	public static void Main() {
		TextLists lists = new TextLists();
		lists.read_english();
		lists.read_old_english();

		MainForm main_form = new MainForm(lists);

		Application.Run(main_form);
	}
}

