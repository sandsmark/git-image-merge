#!/bin/bash
echo '[diff "image"]' >> ~/.gitconfig
echo '    command = git-image-merge' >> ~/.gitconfig
echo '[merge "image"]' >> ~/.gitconfig
echo '    cmd = git-image-merge \"$LOCAL\" \"$REMOTE\" \"$MERGED\"' >> ~/.gitconfig
echo '    trustExitCode = true' >> ~/.gitconfig

echo '*.gif diff=image merge=image' >> ~/.gitattributes
echo '*.jpeg diff=image merge=image' >> ~/.gitattributes
echo '*.jpg diff=image merge=image' >> ~/.gitattributes
echo '*.png diff=image merge=image' >> ~/.gitattributes
