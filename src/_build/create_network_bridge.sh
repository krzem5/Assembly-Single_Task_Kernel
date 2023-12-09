#!/bin/bash
ip link add name br1 type bridge
ip addr add 172.20.0.1/16 dev br1
ip link set br1 up
ifconfig br1 172.20.0.1 netmask 255.255.0.0 up
dnsmasq --strict-order --listen-address=172.20.0.1 --except-interface=lo --interface=br1 --bind-interfaces --dhcp-range=172.20.0.2,172.20.255.254 --conf-file="" --dhcp-no-override
modprobe tun
[[ ! -d /etc/qemu ]]&&mkdir /etc/qemu
echo allow virbr0 > /etc/qemu/bridge.conf
chmod +s /usr/lib/qemu/qemu-bridge-helper
sysctl net.ipv4.ip_forward=1
sysctl net.ipv6.conf.default.forwarding=1
sysctl net.ipv6.conf.all.forwarding=1
interface=$(ip addr|awk '/state UP/ {print $2}'|sed 's/.$//')
iptables -t nat -A POSTROUTING -o "$interface" -j MASQUERADE
iptables -A FORWARD -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT
iptables -A FORWARD -i tap0 -o "$interface" -j ACCEPT
