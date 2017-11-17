%define workingDir %(pwd)
%define _topdir %{workingDir}/rpm
%define kernel_version %(uname -r)

Summary: CMS Emu local DAQ Gbit and peripheral crate VME drivers for kernel %{kernel_version} based on the igb module for the Intel dual port NIC model I350-F2
Name: emu-igb_emu
Version: 2.1.12
Release: 1.slc6
License: none
Group: none
URL: none
Source0: cvs
BuildRoot: /tmp/%{name}-%{version}-%{release}-root


%description


%build


%pre

%install
mkdir -p $RPM_BUILD_ROOT/usr/local/bin/igb_emu
cp %{workingDir}/igb_emu/eth_hook_2_daq/ddu/eth_hook_2_ddu.ko $RPM_BUILD_ROOT/usr/local/bin/igb_emu
cp %{workingDir}/igb_emu/eth_hook_3_daq/ddu/eth_hook_3_ddu.ko $RPM_BUILD_ROOT/usr/local/bin/igb_emu
cp %{workingDir}/igb_emu/eth_hook_4_daq/ddu/eth_hook_4_ddu.ko $RPM_BUILD_ROOT/usr/local/bin/igb_emu
cp %{workingDir}/igb_emu/eth_hook_5_daq/ddu/eth_hook_5_ddu.ko $RPM_BUILD_ROOT/usr/local/bin/igb_emu
cp %{workingDir}/igb_emu/eth_hook_2_daq/dmb/eth_hook_2_dmb.ko $RPM_BUILD_ROOT/usr/local/bin/igb_emu
cp %{workingDir}/igb_emu/eth_hook_3_daq/dmb/eth_hook_3_dmb.ko $RPM_BUILD_ROOT/usr/local/bin/igb_emu
cp %{workingDir}/igb_emu/eth_hook_4_daq/dmb/eth_hook_4_dmb.ko $RPM_BUILD_ROOT/usr/local/bin/igb_emu
cp %{workingDir}/igb_emu/eth_hook_5_daq/dmb/eth_hook_5_dmb.ko $RPM_BUILD_ROOT/usr/local/bin/igb_emu
cp %{workingDir}/igb_emu/eth_hook_2_vme/eth_hook_2_vme.ko $RPM_BUILD_ROOT/usr/local/bin/igb_emu
cp %{workingDir}/igb_emu/eth_hook_3_vme/eth_hook_3_vme.ko $RPM_BUILD_ROOT/usr/local/bin/igb_emu
cp %{workingDir}/igb_emu/eth_hook_4_vme/eth_hook_4_vme.ko $RPM_BUILD_ROOT/usr/local/bin/igb_emu
cp %{workingDir}/igb_emu/eth_hook_5_vme/eth_hook_5_vme.ko $RPM_BUILD_ROOT/usr/local/bin/igb_emu
cp %{workingDir}/igb_emu/igb-5.1.2/src/igb_emu.ko         $RPM_BUILD_ROOT/usr/local/bin/igb_emu
cp %{workingDir}/script/load_igb_emu.sh                   $RPM_BUILD_ROOT/usr/local/bin/igb_emu
touch %{_topdir}/BUILD/ChangeLog
touch %{_topdir}/BUILD/README
touch %{_topdir}/BUILD/MAINTAINER

%clean
[[ ${RPM_BUILD_ROOT} != "/" ]] && rm -rf $RPM_BUILD_ROOT


%files
%defattr(744,root,root,-)
/usr/local/bin/igb_emu
# Files required by Quattor
%defattr(644,root,root,755)
%doc MAINTAINER ChangeLog README

%post
# Have load_igb_emu.sh invoked on booting
[[ -f /etc/rc.d/rc.local ]] && sed -i -e "/\/usr\/local\/bin\/igb_emu\/load_igb_emu.sh/d" /etc/rc.d/rc.local || true
echo "[[ -x /usr/local/bin/igb_emu/load_igb_emu.sh ]] && /usr/local/bin/igb_emu/load_igb_emu.sh > /var/log/load_igb_emu.log 2>&1" >> /etc/rc.d/rc.local

# Load new modules
/usr/local/bin/igb_emu/load_igb_emu.sh || true

%preun
if [[ $1 -eq 0 ]]; then
   echo "Definitive uninstall of this package. Cleaning up."

   # Unload modules
   [[ $(/sbin/lsmod | grep -c igb_emu) -eq 0 ]] || /sbin/modprobe -r igb_emu || true

   # Remove modules from /lib/modules
   rm -f /lib/modules/%{kernel_version}/kernel/drivers/net/igb/igb_emu.ko
   rm -f /lib/modules/%{kernel_version}/kernel/drivers/net/igb/eth_hook_*.ko

   # Update module dependencies
   /sbin/depmod

   # Stop loading daq drivers at boot time.
   [[ -f /etc/rc.d/rc.local ]] && sed -i -e "/\/usr\/local\/bin\/igb_emu\/load_igb_emu.sh/d" /etc/rc.d/rc.local
fi

%postun
