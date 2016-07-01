- `memory_record.cpp`: toggle "m" to start controlling the character. Toggle
	"m" again to stop controlling and press "j" to take a simultaneous screen and
  RAM snapshot, named `000.png` and `000.ram`, in the current directory.

- `format_ram.py`: a script that treats the files it receives as RAM snapshots,
	and prints only the bytes that have changed between the snapshots. Used for
reverse engineering with `memory_record`. Usage: `./format_ram.py <.ram files>`.

- `heuristic_stop.cpp`: play the game with options instead of primitive
	actions. Press the arrow and space keys corresponding to the option, then
  press "c" to execute it. Press "q" to quit.
