Name:           pswmgr
Version:        0.1.0
Release:        1%{?dist}
Summary:        Password manager binaries

License:        GPL-3.0
URL:            https://github.com/viperman1271/%{name}
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc gcc-c++ make unzip curl libtool openssl openssl-devel

%description
Password Manager that uses a client/server architecture to store encrypted passwords remotely

%prep
%setup -q

%build
cd server
make %{?_smp_mflags}
cd ../cli
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
cd server
%make_install
cd ../cli
%make_install

%pre
groupadd -rf pswmgr
id -u pswmgr &>/dev/null || useradd -r -d /etc/pswmgr/ -g pswmgr -s /sbin/nologin pswmgr

%post
chown pswmgr: /etc/%{name}/%{name}d.conf
chown pswmgr: /usr/bin/%{name}d

%files
%doc
/usr/bin/%{name}d
/etc/pswmgr/pswmgrd.conf
/usr/bin/%{name}
/etc/pswmgr/pswmgr.conf

%changelog
* Mon Feb 13 2017 Michael Filion <mfilion@mikefilion.com> 0.1.0-1
- Initial package for CentOS
