# Solution — 01-tool-coverage

Leave ready uninitialized in buggy_create. test_suite_path never
calls it; covered_path does. That is why make valgrind on tests can
be clean while the game is not.

Do **not** delete memset from production entity_create_player.
