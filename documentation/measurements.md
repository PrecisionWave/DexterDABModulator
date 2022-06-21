# Dexter RF output
## Third order intercept point
`iio_attr -d dexter_dsp_tx dc0 10000`

`iio_attr -d dexter_dsp_tx dc1 10000`

`iio_attr -d dexter_dsp_tx -- frequency0 -5300000`

`iio_attr -d dexter_dsp_tx -- frequency0 -4200000`

![TOI Measurement](pictures/TOI_dexter.png)

# Single tone
`iio_attr -d dexter_dsp_tx dc0 46000`

`iio_attr -d dexter_dsp_tx -- frequency0 -4800000`

![10M-1G](pictures/single_tone_w.png)
![50M span](pictures/single_tone_m.png)
![2M span](pictures/single_tone_w.png)
