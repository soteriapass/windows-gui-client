rpmdev-setuptree
cd ~/rpmbuild/SOURCES
git clone https://github.com/viperman1271/pswmgr.git
cp ~/rpmbuild/SOURCES/pswmgr/build/centos/pswmgr.spec ~/rpmbuild/SPECS/.
mv pswmgr pswmgr-$(curl -L https://pswmgr.mikefilion.com/release)
tar -czvf pswmgr-$(curl -L https://pswmgr.mikefilion.com/release).tar.gz pswmgr-$(curl -L https://pswmgr.mikefilion.com/release)/
rm -fr pswmgr-$(curl -L https://pswmgr.mikefilion.com/release)/
cd ~/rpmbuild/SPECS/.
rpmbuild -ba pswmgr.spec
