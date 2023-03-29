clc;
close all;
clear all;
clk=163.84e6;
fs_dac = clk/2;
fs_dab = 2048e3;
bw_dab = 1536e3; % DAB Bandwidth = 1536kHz

dds_phasewidth = 25;

tx_dds_freq =10e6;
tx_inc = mod(round(tx_dds_freq*2^dds_phasewidth/clk*2),2^dds_phasewidth);

hb_interpolation = 8;
fir_interpolation = 5;
interp = fir_interpolation*hb_interpolation;

% coefficients for all three FIR halfband interpolators
d_halfband = fdesign.halfband('N,TW', 26, 0.5);
Hd_halfband = design(d_halfband,'equiripple','SystemObject',true);
%fvtool(Hd_halfband)
hb_coef = Hd_halfband.Numerator;

% coefficients for all the FIR x5 interpolators
d_fir = fdesign.lowpass('N,Fp,Fst', 120, bw_dab/fs_dac*hb_interpolation, -bw_dab/fs_dac*hb_interpolation+2*fs_dab/fs_dac*hb_interpolation);
Hd_fir = design(d_fir,'equiripple','SystemObject',true);
%fvtool(Hd_fir)
fir_coef = Hd_fir.Numerator;

f_stimulus = [100000 700000 768000];
a_stimulus = [0.2 0.2 0.2];