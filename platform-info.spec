Name: platform-info-binding
Version: 9.0.2
Release: 1%{?dist}
License:  APL2.0
Summary: Gps service set to be used in the redpesk
URL: https://git.ovh.iot/redpesk/redpesk-common/platform-info-binding
Source: %{name}-%{version}.tar.gz

%global _afmappdir %{_prefix}/redpesk
%global coverage_dir %{_libexecdir}/redtest/%{name}/coverage_data

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  afb-cmake-modules
BuildRequires:  pkgconfig(json-c)
BuildRequires:  pkgconfig(afb-binding)
BuildRequires:  pkgconfig(afb-libhelpers)
BuildRequires:  pkgconfig(afb-libcontroller)
BuildRequires:  pkgconfig(libsystemd) >= 222
BuildRequires:  pkgconfig(afb-helpers4)
BuildRequires:  pkgconfig(libevdev)
BuildRequires:  lcov
BuildRequires:  pkgconfig(liburcu)
BuildRequires:  pkgconfig(librp-utils-json-c)


%if 0%{?suse_version}
BuildRequires:  libdb-4_8-devel
%else
BuildRequires:  libdb-devel
%endif

Requires:       afb-binder

%description
This binding reads system and platform information and exposes them through its API

%package redtest
Summary: redtest package (coverage build)
Requires: lcov
%description redtest
This package contains binaries built with coverage instrumentation.

%prep
%autosetup -p 1

%build
# Build (no coverage)
mkdir build-no-coverage && cd build-no-coverage
%cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DAFM_APP_DIR=%{_afmappdir} ..
%cmake_build
cd ..

# Build coverage (with coverage flags)
mkdir build-coverage && cd build-coverage
%cmake \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_FLAGS="--coverage -fPIC" \
  -DCMAKE_CXX_FLAGS="--coverage -fPIC" \
  -DAFM_APP_DIR=%{coverage_dir} ..
%cmake_build
cd ..

%install
# Install (base package)
cd build-no-coverage
%cmake_install
cd ..

# Install coverage build (for redtest package)
cd build-coverage
%cmake_install

# Copy the coverage files (.gcno) into the coverage_data directory for redtest
find . -name "*.gcno" -exec cp --parents {} %{buildroot}%{coverage_dir}/ \;
cd ..

%files
%defattr(-,root,root)
%dir %{_afmappdir}/%{name}
%{_afmappdir}/%{name}/lib/
%{_afmappdir}/%{name}/.rpconfig/
%{_afmappdir}/%{name}/var/afm_packages_installed.sh
%{_afmappdir}/%{name}/var/packages_installed.sh
%{_afmappdir}/%{name}/var/packages_number.sh


%files redtest
%defattr(-,root,root)
%{_libexecdir}/redtest/%{name}/run-redtest
%{_libexecdir}/redtest/%{name}/test-basic.py
%{_libexecdir}/redtest/%{name}/test-event.py
%{coverage_dir}


