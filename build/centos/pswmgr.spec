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

%install
rm -rf $RPM_BUILD_ROOT
cd server
%make_install

%files
%doc
/usr/bin/%{name}d
/etc/pswmgr/pswmgrd.conf

%changelog
* Mon Feb 13 2017 Michael Filion <mfilion@mikefilion.com> 0.1-1
- Initial package for CentOS
