// T-maze display.

package mona;

import java.awt.*;
import java.awt.geom.*;
import java.util.*;
import javax.swing.*;

// T-maze display.
public class TmazeDisplay extends JPanel {
	// Special maze junctions.
	static final int START = 0;
	static final int GOAL  = 1;

	// Sizes.
	static final int JUNCTION_SIZE             = 40;
	static final int VERTICAL_CONNECTOR_SIZE   = 40;
	static final int HORIZONTAL_CONNECTOR_SIZE = 160;
	Size        minSize;
	Size        maze_size;

	// Font.
	Font font;

	// Mazes.
	Vector<Tmaze> mazes;

	// Constructor.
	public TmazeDisplay(Size minSize) {
		this.minSize = minSize;
		font         = new Font("Courier", Font.PLAIN, 12);
	}


	// Load mazes.
	public void Load(Vector<Tmaze> mazes) {
		int   i, j, k, m, n;
		Tmaze maze;
		Tmaze.Junction junction;
		this.mazes = mazes;
		// Determine maze display size.
		maze_size = new Size(HORIZONTAL_CONNECTOR_SIZE, VERTICAL_CONNECTOR_SIZE);
		j        = (HORIZONTAL_CONNECTOR_SIZE / 2) + (JUNCTION_SIZE / 2);

		for (i = 0; i < mazes.GetCount(); i++) {
			maze = mazes.Get(i);

			for (k = m = n = 0; k < maze.path.GetCount(); k++) {
				junction = maze.path.Get(k);

				if (junction.mark == GOAL)
					break;

				if (junction.direction == 0)
					m--;
				else
					m++;

				if (n < Math.abs(m))
					n = Math.abs(m);
			}

			n = (n * j * 2) + HORIZONTAL_CONNECTOR_SIZE + JUNCTION_SIZE;

			if (maze_size.width < n)
				maze_size.width = n;
		}

		if (maze_size.width < minSize.width)
			maze_size.width = minSize.width;

		j = VERTICAL_CONNECTOR_SIZE + (JUNCTION_SIZE / 2);

		for (i = 0; i < mazes.GetCount(); i++) {
			maze             = mazes.Get(i);
			maze_size.height += (maze.path.GetCount() * j) + (VERTICAL_CONNECTOR_SIZE * 2);
		}

		if (maze_size.height < minSize.height)
			maze_size.height = minSize.height;

		setPreferredSize(maze_size);
	}


	// Draw the maze.
	public void paint(Graphics g) {
		int         i, j, k, x, y, fw, fh;
		FontMetrics fontMetrics;
		Tmaze maze;
		Tmaze.Junction junction;
		g.setFont(font);
		fontMetrics = g.getFontMetrics();
		fw          = fontMetrics.getMaxAdvance() / 8;
		fh          = fontMetrics.GetHeight() / 4;
		g.setColor(Color.white);
		g.fillRect(0, 0, maze_size.width, maze_size.height);
		g.setColor(Color.black);
		x = maze_size.width / 2;
		y = VERTICAL_CONNECTOR_SIZE + (JUNCTION_SIZE / 2);

		for (i = 0; i < mazes.GetCount(); i++) {
			x    = maze_size.width / 2;
			maze = mazes.Get(i);

			for (j = 0; j < maze.path.GetCount(); j++) {
				junction = maze.path.Get(j);

				if (junction.highlight) {
					if (junction.choice != -1) {
						if (junction.choice == junction.direction)
							g.setColor(Color.green);
						else
							g.setColor(Color.red);
					}
					else
						g.setColor(Color.lightGray);

					g.fillRect(x - (JUNCTION_SIZE / 2), y - (JUNCTION_SIZE / 2),
							   JUNCTION_SIZE, JUNCTION_SIZE);
					g.setColor(Color.black);
				}

				g.drawRect(x - (JUNCTION_SIZE / 2), y - (JUNCTION_SIZE / 2),
						   JUNCTION_SIZE, JUNCTION_SIZE);

				switch (junction.mark) {
				case START:
					g.drawString("s", x - fw, y + fh);
					break;

				case GOAL:
					g.drawString("g", x - fw, y + fh);
					break;

				default:
					g.drawString(junction.mark + "", x - fw, y + fh);
					break;
				}

				y += (JUNCTION_SIZE / 2);

				if (junction.mark != GOAL) {
					g.drawLine(x, y, x, y + VERTICAL_CONNECTOR_SIZE);
					y += VERTICAL_CONNECTOR_SIZE;
					g.drawLine(x - (HORIZONTAL_CONNECTOR_SIZE / 2), y,
							   x + (HORIZONTAL_CONNECTOR_SIZE / 2), y);

					if (junction.choice != -1) {
						if (junction.choice == 0)
							g.drawString("left", x - (HORIZONTAL_CONNECTOR_SIZE / 4) - 12, y - 5);
						else
							g.drawString("right", x + (HORIZONTAL_CONNECTOR_SIZE / 4) - 15, y - 5);
					}
					else {
						g.drawString(String.format("%f", junction.probability), x - (HORIZONTAL_CONNECTOR_SIZE / 4) - 28, y - 5);
						g.drawString(String.format("%f", (1.0 - junction.probability)), x + (HORIZONTAL_CONNECTOR_SIZE / 4) - 28, y - 5);
					}

					if (junction.annotations != null) {
						for (k = 0; k < junction.annotations.GetCount(); k++) {
							g.drawString(junction.annotations.Get(k),
										 (x + 4) - (HORIZONTAL_CONNECTOR_SIZE / 2), ((k + 1) * 15) + y);
						}
					}

					if (junction.direction == 0) {
						x -= (HORIZONTAL_CONNECTOR_SIZE / 2);
						x -= (JUNCTION_SIZE / 2);
					}
					else {
						x += (HORIZONTAL_CONNECTOR_SIZE / 2);
						x += (JUNCTION_SIZE / 2);
					}
				}
			}

			y += (VERTICAL_CONNECTOR_SIZE * 2);
		}
	}
}
