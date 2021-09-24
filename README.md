git-image-merge
---------------

Simple visual diff and merging tool for images. Basically shows the
alternatives side by side (when running `git diff` or `git merge`), and in merge
mode allows you to double click on the correct one.

If you want to actually find and inspect differences between images, try
[diffimg](https://github.com/sandsmark/diffimg).


usage
-----

qmake && make, and put the executable somewhere in your PATH where git will find it.

Then configure git to use it, I haven't tested install.sh, but you get the gist
of what you need to do.


