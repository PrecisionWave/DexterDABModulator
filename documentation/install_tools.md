# Dexter Tools
Dexter Tools installation instructions.

## Install Dexter Tools
To build the boot.bin the following input files must be built first:
1. Establish ssh-connection to the Dexter Device (password is 'analog')
```
ssh root@<IP>
```

2. Clone DexterTools git repository to /root/tools
```
git clone https://github.com/PrecisionWave/DexterTools.git /root/tools/
```

3. Run install script
```
cd /root/tools/
./install-tools.sh
```
