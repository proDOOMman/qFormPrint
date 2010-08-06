#BuildRequires: qt-devel

Summary: chineese counter

Summary(ru): Заполенение форм учета иностранных граждан

%define version 0.011

License: GPL

Group: Development/Languages

Name: migrant

Prefix: /usr

Provides: migrant

Release: 1

Source: migrant-%{version}.tar.bz2

URL: http://127.0.0.1/

Version: %{version}

Buildroot: /tmp/migrant

%description
This is very good tool

%description -l ru
Программа кроссплатформенная, написана на QT4

%prep

%setup -q

%build

qmake -config release
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin
cp Migrant $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/share/applications
cp migrant.desktop $RPM_BUILD_ROOT/usr/share/applications
mkdir -p $RPM_BUILD_ROOT/usr/share/Migrant/data/
cp data/* $RPM_BUILD_ROOT/usr/share/Migrant/data/

%clean

rm -rf $RPM_BUILD_ROOT

%files

%defattr(-,root,root)
/usr/bin/Migrant
/usr/share/applications/migrant.desktop
/usr/share/Migrant/data/*

