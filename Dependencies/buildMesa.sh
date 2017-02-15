pip install Mako
wget ftp://ftp.freedesktop.org/pub/mesa/13.0.4/mesa-13.0.4.tar.gz
tar xzf mesa-13.0.4.tar.gz
cd mesa-13.0.4
./configure --enable-opengl --disable-gles1 --disable-gles2 --with-gallium-drivers=swrast --disable-dri --with-dri-drivers= --disable-egl --with-egl-platforms= --disable-gbm --disable-glx --enable-gallium-osmesa --enable-texture-float --with-llvm-prefix=/usr --disable-va --disable-xvmc --disable-vdpau --prefix=/usr
make -j 4
sudo make install
cd ..
