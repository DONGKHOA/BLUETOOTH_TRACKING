skipUTF8 proof
==============

This directory contains a memory safety proof for `skipUTF8`.
The proof runs in a few seconds and provides 100% coverage.

To run the proof.
* Add `cbmc`, `goto-cc`, `goto-instrument`, `goto-analyzer`, and `cbmc-viewer`
  to your path;
* Run `make`;
* Open `html/index.html` in a web browser.
