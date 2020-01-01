.section .progmem.assets
.balign 256
.global cosinetable
.global cosinetable_lsb
.global cosinetable_msb
.type cosinetable, @object
.type cosinetable_lsb, @object
.type cosinetable_msb, @object
cosinetable:
cosinetable_lsb:
  .byte lo8( 32767)  ; i=  0  theta=0.00000  cos(theta)*32767=32767.000
  .byte lo8( 32757)  ; i=  1  theta=0.02454  cos(theta)*32767=32757.131
  .byte lo8( 32728)  ; i=  2  theta=0.04909  cos(theta)*32767=32727.531
  .byte lo8( 32678)  ; i=  3  theta=0.07363  cos(theta)*32767=32678.216
  .byte lo8( 32609)  ; i=  4  theta=0.09817  cos(theta)*32767=32609.218
  .byte lo8( 32521)  ; i=  5  theta=0.12272  cos(theta)*32767=32520.577
  .byte lo8( 32412)  ; i=  6  theta=0.14726  cos(theta)*32767=32412.347
  .byte lo8( 32285)  ; i=  7  theta=0.17181  cos(theta)*32767=32284.593
  .byte lo8( 32137)  ; i=  8  theta=0.19635  cos(theta)*32767=32137.391
  .byte lo8( 31971)  ; i=  9  theta=0.22089  cos(theta)*32767=31970.832
  .byte lo8( 31785)  ; i= 10  theta=0.24544  cos(theta)*32767=31785.014
  .byte lo8( 31580)  ; i= 11  theta=0.26998  cos(theta)*32767=31580.050
  .byte lo8( 31356)  ; i= 12  theta=0.29452  cos(theta)*32767=31356.064
  .byte lo8( 31113)  ; i= 13  theta=0.31907  cos(theta)*32767=31113.190
  .byte lo8( 30852)  ; i= 14  theta=0.34361  cos(theta)*32767=30851.574
  .byte lo8( 30571)  ; i= 15  theta=0.36816  cos(theta)*32767=30571.375
  .byte lo8( 30273)  ; i= 16  theta=0.39270  cos(theta)*32767=30272.761
  .byte lo8( 29956)  ; i= 17  theta=0.41724  cos(theta)*32767=29955.911
  .byte lo8( 29621)  ; i= 18  theta=0.44179  cos(theta)*32767=29621.017
  .byte lo8( 29268)  ; i= 19  theta=0.46633  cos(theta)*32767=29268.281
  .byte lo8( 28898)  ; i= 20  theta=0.49087  cos(theta)*32767=28897.914
  .byte lo8( 28510)  ; i= 21  theta=0.51542  cos(theta)*32767=28510.140
  .byte lo8( 28105)  ; i= 22  theta=0.53996  cos(theta)*32767=28105.193
  .byte lo8( 27683)  ; i= 23  theta=0.56450  cos(theta)*32767=27683.317
  .byte lo8( 27245)  ; i= 24  theta=0.58905  cos(theta)*32767=27244.765
  .byte lo8( 26790)  ; i= 25  theta=0.61359  cos(theta)*32767=26789.802
  .byte lo8( 26319)  ; i= 26  theta=0.63814  cos(theta)*32767=26318.701
  .byte lo8( 25832)  ; i= 27  theta=0.66268  cos(theta)*32767=25831.747
  .byte lo8( 25329)  ; i= 28  theta=0.68722  cos(theta)*32767=25329.234
  .byte lo8( 24811)  ; i= 29  theta=0.71177  cos(theta)*32767=24811.462
  .byte lo8( 24279)  ; i= 30  theta=0.73631  cos(theta)*32767=24278.746
  .byte lo8( 23731)  ; i= 31  theta=0.76085  cos(theta)*32767=23731.404
  .byte lo8( 23170)  ; i= 32  theta=0.78540  cos(theta)*32767=23169.768
  .byte lo8( 22594)  ; i= 33  theta=0.80994  cos(theta)*32767=22594.175
  .byte lo8( 22005)  ; i= 34  theta=0.83449  cos(theta)*32767=22004.972
  .byte lo8( 21403)  ; i= 35  theta=0.85903  cos(theta)*32767=21402.515
  .byte lo8( 20787)  ; i= 36  theta=0.88357  cos(theta)*32767=20787.165
  .byte lo8( 20159)  ; i= 37  theta=0.90812  cos(theta)*32767=20159.294
  .byte lo8( 19519)  ; i= 38  theta=0.93266  cos(theta)*32767=19519.279
  .byte lo8( 18868)  ; i= 39  theta=0.95720  cos(theta)*32767=18867.507
  .byte lo8( 18204)  ; i= 40  theta=0.98175  cos(theta)*32767=18204.370
  .byte lo8( 17530)  ; i= 41  theta=1.00629  cos(theta)*32767=17530.267
  .byte lo8( 16846)  ; i= 42  theta=1.03084  cos(theta)*32767=16845.605
  .byte lo8( 16151)  ; i= 43  theta=1.05538  cos(theta)*32767=16150.795
  .byte lo8( 15446)  ; i= 44  theta=1.07992  cos(theta)*32767=15446.257
  .byte lo8( 14732)  ; i= 45  theta=1.10447  cos(theta)*32767=14732.414
  .byte lo8( 14010)  ; i= 46  theta=1.12901  cos(theta)*32767=14009.698
  .byte lo8( 13279)  ; i= 47  theta=1.15355  cos(theta)*32767=13278.542
  .byte lo8( 12539)  ; i= 48  theta=1.17810  cos(theta)*32767=12539.388
  .byte lo8( 11793)  ; i= 49  theta=1.20264  cos(theta)*32767=11792.681
  .byte lo8( 11039)  ; i= 50  theta=1.22718  cos(theta)*32767=11038.870
  .byte lo8( 10278)  ; i= 51  theta=1.25173  cos(theta)*32767=10278.410
  .byte lo8(  9512)  ; i= 52  theta=1.27627  cos(theta)*32767=9511.758
  .byte lo8(  8739)  ; i= 53  theta=1.30082  cos(theta)*32767=8739.377
  .byte lo8(  7962)  ; i= 54  theta=1.32536  cos(theta)*32767=7961.732
  .byte lo8(  7179)  ; i= 55  theta=1.34990  cos(theta)*32767=7179.290
  .byte lo8(  6393)  ; i= 56  theta=1.37445  cos(theta)*32767=6392.525
  .byte lo8(  5602)  ; i= 57  theta=1.39899  cos(theta)*32767=5601.908
  .byte lo8(  4808)  ; i= 58  theta=1.42353  cos(theta)*32767=4807.917
  .byte lo8(  4011)  ; i= 59  theta=1.44808  cos(theta)*32767=4011.031
  .byte lo8(  3212)  ; i= 60  theta=1.47262  cos(theta)*32767=3211.728
  .byte lo8(  2410)  ; i= 61  theta=1.49717  cos(theta)*32767=2410.490
  .byte lo8(  1608)  ; i= 62  theta=1.52171  cos(theta)*32767=1607.800
  .byte lo8(   804)  ; i= 63  theta=1.54625  cos(theta)*32767=804.142
  .byte lo8(     0)  ; i= 64  theta=1.57080  cos(theta)*32767=0.000
  .byte lo8(  -804)  ; i= 65  theta=1.59534  cos(theta)*32767=-804.142
  .byte lo8( -1608)  ; i= 66  theta=1.61988  cos(theta)*32767=-1607.800
  .byte lo8( -2410)  ; i= 67  theta=1.64443  cos(theta)*32767=-2410.490
  .byte lo8( -3212)  ; i= 68  theta=1.66897  cos(theta)*32767=-3211.728
  .byte lo8( -4011)  ; i= 69  theta=1.69351  cos(theta)*32767=-4011.031
  .byte lo8( -4808)  ; i= 70  theta=1.71806  cos(theta)*32767=-4807.917
  .byte lo8( -5602)  ; i= 71  theta=1.74260  cos(theta)*32767=-5601.908
  .byte lo8( -6393)  ; i= 72  theta=1.76715  cos(theta)*32767=-6392.525
  .byte lo8( -7179)  ; i= 73  theta=1.79169  cos(theta)*32767=-7179.290
  .byte lo8( -7962)  ; i= 74  theta=1.81623  cos(theta)*32767=-7961.732
  .byte lo8( -8739)  ; i= 75  theta=1.84078  cos(theta)*32767=-8739.377
  .byte lo8( -9512)  ; i= 76  theta=1.86532  cos(theta)*32767=-9511.758
  .byte lo8(-10278)  ; i= 77  theta=1.88986  cos(theta)*32767=-10278.410
  .byte lo8(-11039)  ; i= 78  theta=1.91441  cos(theta)*32767=-11038.870
  .byte lo8(-11793)  ; i= 79  theta=1.93895  cos(theta)*32767=-11792.681
  .byte lo8(-12539)  ; i= 80  theta=1.96350  cos(theta)*32767=-12539.388
  .byte lo8(-13279)  ; i= 81  theta=1.98804  cos(theta)*32767=-13278.542
  .byte lo8(-14010)  ; i= 82  theta=2.01258  cos(theta)*32767=-14009.698
  .byte lo8(-14732)  ; i= 83  theta=2.03713  cos(theta)*32767=-14732.414
  .byte lo8(-15446)  ; i= 84  theta=2.06167  cos(theta)*32767=-15446.257
  .byte lo8(-16151)  ; i= 85  theta=2.08621  cos(theta)*32767=-16150.795
  .byte lo8(-16846)  ; i= 86  theta=2.11076  cos(theta)*32767=-16845.605
  .byte lo8(-17530)  ; i= 87  theta=2.13530  cos(theta)*32767=-17530.267
  .byte lo8(-18204)  ; i= 88  theta=2.15984  cos(theta)*32767=-18204.370
  .byte lo8(-18868)  ; i= 89  theta=2.18439  cos(theta)*32767=-18867.507
  .byte lo8(-19519)  ; i= 90  theta=2.20893  cos(theta)*32767=-19519.279
  .byte lo8(-20159)  ; i= 91  theta=2.23348  cos(theta)*32767=-20159.294
  .byte lo8(-20787)  ; i= 92  theta=2.25802  cos(theta)*32767=-20787.165
  .byte lo8(-21403)  ; i= 93  theta=2.28256  cos(theta)*32767=-21402.515
  .byte lo8(-22005)  ; i= 94  theta=2.30711  cos(theta)*32767=-22004.972
  .byte lo8(-22594)  ; i= 95  theta=2.33165  cos(theta)*32767=-22594.175
  .byte lo8(-23170)  ; i= 96  theta=2.35619  cos(theta)*32767=-23169.768
  .byte lo8(-23731)  ; i= 97  theta=2.38074  cos(theta)*32767=-23731.404
  .byte lo8(-24279)  ; i= 98  theta=2.40528  cos(theta)*32767=-24278.746
  .byte lo8(-24811)  ; i= 99  theta=2.42983  cos(theta)*32767=-24811.462
  .byte lo8(-25329)  ; i=100  theta=2.45437  cos(theta)*32767=-25329.234
  .byte lo8(-25832)  ; i=101  theta=2.47891  cos(theta)*32767=-25831.747
  .byte lo8(-26319)  ; i=102  theta=2.50346  cos(theta)*32767=-26318.701
  .byte lo8(-26790)  ; i=103  theta=2.52800  cos(theta)*32767=-26789.802
  .byte lo8(-27245)  ; i=104  theta=2.55254  cos(theta)*32767=-27244.765
  .byte lo8(-27683)  ; i=105  theta=2.57709  cos(theta)*32767=-27683.317
  .byte lo8(-28105)  ; i=106  theta=2.60163  cos(theta)*32767=-28105.193
  .byte lo8(-28510)  ; i=107  theta=2.62618  cos(theta)*32767=-28510.140
  .byte lo8(-28898)  ; i=108  theta=2.65072  cos(theta)*32767=-28897.914
  .byte lo8(-29268)  ; i=109  theta=2.67526  cos(theta)*32767=-29268.281
  .byte lo8(-29621)  ; i=110  theta=2.69981  cos(theta)*32767=-29621.017
  .byte lo8(-29956)  ; i=111  theta=2.72435  cos(theta)*32767=-29955.911
  .byte lo8(-30273)  ; i=112  theta=2.74889  cos(theta)*32767=-30272.761
  .byte lo8(-30571)  ; i=113  theta=2.77344  cos(theta)*32767=-30571.375
  .byte lo8(-30852)  ; i=114  theta=2.79798  cos(theta)*32767=-30851.574
  .byte lo8(-31113)  ; i=115  theta=2.82252  cos(theta)*32767=-31113.190
  .byte lo8(-31356)  ; i=116  theta=2.84707  cos(theta)*32767=-31356.064
  .byte lo8(-31580)  ; i=117  theta=2.87161  cos(theta)*32767=-31580.050
  .byte lo8(-31785)  ; i=118  theta=2.89616  cos(theta)*32767=-31785.014
  .byte lo8(-31971)  ; i=119  theta=2.92070  cos(theta)*32767=-31970.832
  .byte lo8(-32137)  ; i=120  theta=2.94524  cos(theta)*32767=-32137.391
  .byte lo8(-32285)  ; i=121  theta=2.96979  cos(theta)*32767=-32284.593
  .byte lo8(-32412)  ; i=122  theta=2.99433  cos(theta)*32767=-32412.347
  .byte lo8(-32521)  ; i=123  theta=3.01887  cos(theta)*32767=-32520.577
  .byte lo8(-32609)  ; i=124  theta=3.04342  cos(theta)*32767=-32609.218
  .byte lo8(-32678)  ; i=125  theta=3.06796  cos(theta)*32767=-32678.216
  .byte lo8(-32728)  ; i=126  theta=3.09251  cos(theta)*32767=-32727.531
  .byte lo8(-32757)  ; i=127  theta=3.11705  cos(theta)*32767=-32757.131
  .byte lo8(-32767)  ; i=128  theta=3.14159  cos(theta)*32767=-32767.000
  .byte lo8(-32757)  ; i=129  theta=3.16614  cos(theta)*32767=-32757.131
  .byte lo8(-32728)  ; i=130  theta=3.19068  cos(theta)*32767=-32727.531
  .byte lo8(-32678)  ; i=131  theta=3.21522  cos(theta)*32767=-32678.216
  .byte lo8(-32609)  ; i=132  theta=3.23977  cos(theta)*32767=-32609.218
  .byte lo8(-32521)  ; i=133  theta=3.26431  cos(theta)*32767=-32520.577
  .byte lo8(-32412)  ; i=134  theta=3.28885  cos(theta)*32767=-32412.347
  .byte lo8(-32285)  ; i=135  theta=3.31340  cos(theta)*32767=-32284.593
  .byte lo8(-32137)  ; i=136  theta=3.33794  cos(theta)*32767=-32137.391
  .byte lo8(-31971)  ; i=137  theta=3.36249  cos(theta)*32767=-31970.832
  .byte lo8(-31785)  ; i=138  theta=3.38703  cos(theta)*32767=-31785.014
  .byte lo8(-31580)  ; i=139  theta=3.41157  cos(theta)*32767=-31580.050
  .byte lo8(-31356)  ; i=140  theta=3.43612  cos(theta)*32767=-31356.064
  .byte lo8(-31113)  ; i=141  theta=3.46066  cos(theta)*32767=-31113.190
  .byte lo8(-30852)  ; i=142  theta=3.48520  cos(theta)*32767=-30851.574
  .byte lo8(-30571)  ; i=143  theta=3.50975  cos(theta)*32767=-30571.375
  .byte lo8(-30273)  ; i=144  theta=3.53429  cos(theta)*32767=-30272.761
  .byte lo8(-29956)  ; i=145  theta=3.55884  cos(theta)*32767=-29955.911
  .byte lo8(-29621)  ; i=146  theta=3.58338  cos(theta)*32767=-29621.017
  .byte lo8(-29268)  ; i=147  theta=3.60792  cos(theta)*32767=-29268.281
  .byte lo8(-28898)  ; i=148  theta=3.63247  cos(theta)*32767=-28897.914
  .byte lo8(-28510)  ; i=149  theta=3.65701  cos(theta)*32767=-28510.140
  .byte lo8(-28105)  ; i=150  theta=3.68155  cos(theta)*32767=-28105.193
  .byte lo8(-27683)  ; i=151  theta=3.70610  cos(theta)*32767=-27683.317
  .byte lo8(-27245)  ; i=152  theta=3.73064  cos(theta)*32767=-27244.765
  .byte lo8(-26790)  ; i=153  theta=3.75518  cos(theta)*32767=-26789.802
  .byte lo8(-26319)  ; i=154  theta=3.77973  cos(theta)*32767=-26318.701
  .byte lo8(-25832)  ; i=155  theta=3.80427  cos(theta)*32767=-25831.747
  .byte lo8(-25329)  ; i=156  theta=3.82882  cos(theta)*32767=-25329.234
  .byte lo8(-24811)  ; i=157  theta=3.85336  cos(theta)*32767=-24811.462
  .byte lo8(-24279)  ; i=158  theta=3.87790  cos(theta)*32767=-24278.746
  .byte lo8(-23731)  ; i=159  theta=3.90245  cos(theta)*32767=-23731.404
  .byte lo8(-23170)  ; i=160  theta=3.92699  cos(theta)*32767=-23169.768
  .byte lo8(-22594)  ; i=161  theta=3.95153  cos(theta)*32767=-22594.175
  .byte lo8(-22005)  ; i=162  theta=3.97608  cos(theta)*32767=-22004.972
  .byte lo8(-21403)  ; i=163  theta=4.00062  cos(theta)*32767=-21402.515
  .byte lo8(-20787)  ; i=164  theta=4.02517  cos(theta)*32767=-20787.165
  .byte lo8(-20159)  ; i=165  theta=4.04971  cos(theta)*32767=-20159.294
  .byte lo8(-19519)  ; i=166  theta=4.07425  cos(theta)*32767=-19519.279
  .byte lo8(-18868)  ; i=167  theta=4.09880  cos(theta)*32767=-18867.507
  .byte lo8(-18204)  ; i=168  theta=4.12334  cos(theta)*32767=-18204.370
  .byte lo8(-17530)  ; i=169  theta=4.14788  cos(theta)*32767=-17530.267
  .byte lo8(-16846)  ; i=170  theta=4.17243  cos(theta)*32767=-16845.605
  .byte lo8(-16151)  ; i=171  theta=4.19697  cos(theta)*32767=-16150.795
  .byte lo8(-15446)  ; i=172  theta=4.22152  cos(theta)*32767=-15446.257
  .byte lo8(-14732)  ; i=173  theta=4.24606  cos(theta)*32767=-14732.414
  .byte lo8(-14010)  ; i=174  theta=4.27060  cos(theta)*32767=-14009.698
  .byte lo8(-13279)  ; i=175  theta=4.29515  cos(theta)*32767=-13278.542
  .byte lo8(-12539)  ; i=176  theta=4.31969  cos(theta)*32767=-12539.388
  .byte lo8(-11793)  ; i=177  theta=4.34423  cos(theta)*32767=-11792.681
  .byte lo8(-11039)  ; i=178  theta=4.36878  cos(theta)*32767=-11038.870
  .byte lo8(-10278)  ; i=179  theta=4.39332  cos(theta)*32767=-10278.410
  .byte lo8( -9512)  ; i=180  theta=4.41786  cos(theta)*32767=-9511.758
  .byte lo8( -8739)  ; i=181  theta=4.44241  cos(theta)*32767=-8739.377
  .byte lo8( -7962)  ; i=182  theta=4.46695  cos(theta)*32767=-7961.732
  .byte lo8( -7179)  ; i=183  theta=4.49150  cos(theta)*32767=-7179.290
  .byte lo8( -6393)  ; i=184  theta=4.51604  cos(theta)*32767=-6392.525
  .byte lo8( -5602)  ; i=185  theta=4.54058  cos(theta)*32767=-5601.908
  .byte lo8( -4808)  ; i=186  theta=4.56513  cos(theta)*32767=-4807.917
  .byte lo8( -4011)  ; i=187  theta=4.58967  cos(theta)*32767=-4011.031
  .byte lo8( -3212)  ; i=188  theta=4.61421  cos(theta)*32767=-3211.728
  .byte lo8( -2410)  ; i=189  theta=4.63876  cos(theta)*32767=-2410.490
  .byte lo8( -1608)  ; i=190  theta=4.66330  cos(theta)*32767=-1607.800
  .byte lo8(  -804)  ; i=191  theta=4.68785  cos(theta)*32767=-804.142
  .byte lo8(     0)  ; i=192  theta=4.71239  cos(theta)*32767=-0.000
  .byte lo8(   804)  ; i=193  theta=4.73693  cos(theta)*32767=804.142
  .byte lo8(  1608)  ; i=194  theta=4.76148  cos(theta)*32767=1607.800
  .byte lo8(  2410)  ; i=195  theta=4.78602  cos(theta)*32767=2410.490
  .byte lo8(  3212)  ; i=196  theta=4.81056  cos(theta)*32767=3211.728
  .byte lo8(  4011)  ; i=197  theta=4.83511  cos(theta)*32767=4011.031
  .byte lo8(  4808)  ; i=198  theta=4.85965  cos(theta)*32767=4807.917
  .byte lo8(  5602)  ; i=199  theta=4.88419  cos(theta)*32767=5601.908
  .byte lo8(  6393)  ; i=200  theta=4.90874  cos(theta)*32767=6392.525
  .byte lo8(  7179)  ; i=201  theta=4.93328  cos(theta)*32767=7179.290
  .byte lo8(  7962)  ; i=202  theta=4.95783  cos(theta)*32767=7961.732
  .byte lo8(  8739)  ; i=203  theta=4.98237  cos(theta)*32767=8739.377
  .byte lo8(  9512)  ; i=204  theta=5.00691  cos(theta)*32767=9511.758
  .byte lo8( 10278)  ; i=205  theta=5.03146  cos(theta)*32767=10278.410
  .byte lo8( 11039)  ; i=206  theta=5.05600  cos(theta)*32767=11038.870
  .byte lo8( 11793)  ; i=207  theta=5.08054  cos(theta)*32767=11792.681
  .byte lo8( 12539)  ; i=208  theta=5.10509  cos(theta)*32767=12539.388
  .byte lo8( 13279)  ; i=209  theta=5.12963  cos(theta)*32767=13278.542
  .byte lo8( 14010)  ; i=210  theta=5.15418  cos(theta)*32767=14009.698
  .byte lo8( 14732)  ; i=211  theta=5.17872  cos(theta)*32767=14732.414
  .byte lo8( 15446)  ; i=212  theta=5.20326  cos(theta)*32767=15446.257
  .byte lo8( 16151)  ; i=213  theta=5.22781  cos(theta)*32767=16150.795
  .byte lo8( 16846)  ; i=214  theta=5.25235  cos(theta)*32767=16845.605
  .byte lo8( 17530)  ; i=215  theta=5.27689  cos(theta)*32767=17530.267
  .byte lo8( 18204)  ; i=216  theta=5.30144  cos(theta)*32767=18204.370
  .byte lo8( 18868)  ; i=217  theta=5.32598  cos(theta)*32767=18867.507
  .byte lo8( 19519)  ; i=218  theta=5.35052  cos(theta)*32767=19519.279
  .byte lo8( 20159)  ; i=219  theta=5.37507  cos(theta)*32767=20159.294
  .byte lo8( 20787)  ; i=220  theta=5.39961  cos(theta)*32767=20787.165
  .byte lo8( 21403)  ; i=221  theta=5.42416  cos(theta)*32767=21402.515
  .byte lo8( 22005)  ; i=222  theta=5.44870  cos(theta)*32767=22004.972
  .byte lo8( 22594)  ; i=223  theta=5.47324  cos(theta)*32767=22594.175
  .byte lo8( 23170)  ; i=224  theta=5.49779  cos(theta)*32767=23169.768
  .byte lo8( 23731)  ; i=225  theta=5.52233  cos(theta)*32767=23731.404
  .byte lo8( 24279)  ; i=226  theta=5.54687  cos(theta)*32767=24278.746
  .byte lo8( 24811)  ; i=227  theta=5.57142  cos(theta)*32767=24811.462
  .byte lo8( 25329)  ; i=228  theta=5.59596  cos(theta)*32767=25329.234
  .byte lo8( 25832)  ; i=229  theta=5.62051  cos(theta)*32767=25831.747
  .byte lo8( 26319)  ; i=230  theta=5.64505  cos(theta)*32767=26318.701
  .byte lo8( 26790)  ; i=231  theta=5.66959  cos(theta)*32767=26789.802
  .byte lo8( 27245)  ; i=232  theta=5.69414  cos(theta)*32767=27244.765
  .byte lo8( 27683)  ; i=233  theta=5.71868  cos(theta)*32767=27683.317
  .byte lo8( 28105)  ; i=234  theta=5.74322  cos(theta)*32767=28105.193
  .byte lo8( 28510)  ; i=235  theta=5.76777  cos(theta)*32767=28510.140
  .byte lo8( 28898)  ; i=236  theta=5.79231  cos(theta)*32767=28897.914
  .byte lo8( 29268)  ; i=237  theta=5.81686  cos(theta)*32767=29268.281
  .byte lo8( 29621)  ; i=238  theta=5.84140  cos(theta)*32767=29621.017
  .byte lo8( 29956)  ; i=239  theta=5.86594  cos(theta)*32767=29955.911
  .byte lo8( 30273)  ; i=240  theta=5.89049  cos(theta)*32767=30272.761
  .byte lo8( 30571)  ; i=241  theta=5.91503  cos(theta)*32767=30571.375
  .byte lo8( 30852)  ; i=242  theta=5.93957  cos(theta)*32767=30851.574
  .byte lo8( 31113)  ; i=243  theta=5.96412  cos(theta)*32767=31113.190
  .byte lo8( 31356)  ; i=244  theta=5.98866  cos(theta)*32767=31356.064
  .byte lo8( 31580)  ; i=245  theta=6.01320  cos(theta)*32767=31580.050
  .byte lo8( 31785)  ; i=246  theta=6.03775  cos(theta)*32767=31785.014
  .byte lo8( 31971)  ; i=247  theta=6.06229  cos(theta)*32767=31970.832
  .byte lo8( 32137)  ; i=248  theta=6.08684  cos(theta)*32767=32137.391
  .byte lo8( 32285)  ; i=249  theta=6.11138  cos(theta)*32767=32284.593
  .byte lo8( 32412)  ; i=250  theta=6.13592  cos(theta)*32767=32412.347
  .byte lo8( 32521)  ; i=251  theta=6.16047  cos(theta)*32767=32520.577
  .byte lo8( 32609)  ; i=252  theta=6.18501  cos(theta)*32767=32609.218
  .byte lo8( 32678)  ; i=253  theta=6.20955  cos(theta)*32767=32678.216
  .byte lo8( 32728)  ; i=254  theta=6.23410  cos(theta)*32767=32727.531
  .byte lo8( 32757)  ; i=255  theta=6.25864  cos(theta)*32767=32757.131
cosinetable_msb:
  .byte hi8( 32767)  ; i=  0  theta=0.00000  cos(theta)*32767=32767.000
  .byte hi8( 32757)  ; i=  1  theta=0.02454  cos(theta)*32767=32757.131
  .byte hi8( 32728)  ; i=  2  theta=0.04909  cos(theta)*32767=32727.531
  .byte hi8( 32678)  ; i=  3  theta=0.07363  cos(theta)*32767=32678.216
  .byte hi8( 32609)  ; i=  4  theta=0.09817  cos(theta)*32767=32609.218
  .byte hi8( 32521)  ; i=  5  theta=0.12272  cos(theta)*32767=32520.577
  .byte hi8( 32412)  ; i=  6  theta=0.14726  cos(theta)*32767=32412.347
  .byte hi8( 32285)  ; i=  7  theta=0.17181  cos(theta)*32767=32284.593
  .byte hi8( 32137)  ; i=  8  theta=0.19635  cos(theta)*32767=32137.391
  .byte hi8( 31971)  ; i=  9  theta=0.22089  cos(theta)*32767=31970.832
  .byte hi8( 31785)  ; i= 10  theta=0.24544  cos(theta)*32767=31785.014
  .byte hi8( 31580)  ; i= 11  theta=0.26998  cos(theta)*32767=31580.050
  .byte hi8( 31356)  ; i= 12  theta=0.29452  cos(theta)*32767=31356.064
  .byte hi8( 31113)  ; i= 13  theta=0.31907  cos(theta)*32767=31113.190
  .byte hi8( 30852)  ; i= 14  theta=0.34361  cos(theta)*32767=30851.574
  .byte hi8( 30571)  ; i= 15  theta=0.36816  cos(theta)*32767=30571.375
  .byte hi8( 30273)  ; i= 16  theta=0.39270  cos(theta)*32767=30272.761
  .byte hi8( 29956)  ; i= 17  theta=0.41724  cos(theta)*32767=29955.911
  .byte hi8( 29621)  ; i= 18  theta=0.44179  cos(theta)*32767=29621.017
  .byte hi8( 29268)  ; i= 19  theta=0.46633  cos(theta)*32767=29268.281
  .byte hi8( 28898)  ; i= 20  theta=0.49087  cos(theta)*32767=28897.914
  .byte hi8( 28510)  ; i= 21  theta=0.51542  cos(theta)*32767=28510.140
  .byte hi8( 28105)  ; i= 22  theta=0.53996  cos(theta)*32767=28105.193
  .byte hi8( 27683)  ; i= 23  theta=0.56450  cos(theta)*32767=27683.317
  .byte hi8( 27245)  ; i= 24  theta=0.58905  cos(theta)*32767=27244.765
  .byte hi8( 26790)  ; i= 25  theta=0.61359  cos(theta)*32767=26789.802
  .byte hi8( 26319)  ; i= 26  theta=0.63814  cos(theta)*32767=26318.701
  .byte hi8( 25832)  ; i= 27  theta=0.66268  cos(theta)*32767=25831.747
  .byte hi8( 25329)  ; i= 28  theta=0.68722  cos(theta)*32767=25329.234
  .byte hi8( 24811)  ; i= 29  theta=0.71177  cos(theta)*32767=24811.462
  .byte hi8( 24279)  ; i= 30  theta=0.73631  cos(theta)*32767=24278.746
  .byte hi8( 23731)  ; i= 31  theta=0.76085  cos(theta)*32767=23731.404
  .byte hi8( 23170)  ; i= 32  theta=0.78540  cos(theta)*32767=23169.768
  .byte hi8( 22594)  ; i= 33  theta=0.80994  cos(theta)*32767=22594.175
  .byte hi8( 22005)  ; i= 34  theta=0.83449  cos(theta)*32767=22004.972
  .byte hi8( 21403)  ; i= 35  theta=0.85903  cos(theta)*32767=21402.515
  .byte hi8( 20787)  ; i= 36  theta=0.88357  cos(theta)*32767=20787.165
  .byte hi8( 20159)  ; i= 37  theta=0.90812  cos(theta)*32767=20159.294
  .byte hi8( 19519)  ; i= 38  theta=0.93266  cos(theta)*32767=19519.279
  .byte hi8( 18868)  ; i= 39  theta=0.95720  cos(theta)*32767=18867.507
  .byte hi8( 18204)  ; i= 40  theta=0.98175  cos(theta)*32767=18204.370
  .byte hi8( 17530)  ; i= 41  theta=1.00629  cos(theta)*32767=17530.267
  .byte hi8( 16846)  ; i= 42  theta=1.03084  cos(theta)*32767=16845.605
  .byte hi8( 16151)  ; i= 43  theta=1.05538  cos(theta)*32767=16150.795
  .byte hi8( 15446)  ; i= 44  theta=1.07992  cos(theta)*32767=15446.257
  .byte hi8( 14732)  ; i= 45  theta=1.10447  cos(theta)*32767=14732.414
  .byte hi8( 14010)  ; i= 46  theta=1.12901  cos(theta)*32767=14009.698
  .byte hi8( 13279)  ; i= 47  theta=1.15355  cos(theta)*32767=13278.542
  .byte hi8( 12539)  ; i= 48  theta=1.17810  cos(theta)*32767=12539.388
  .byte hi8( 11793)  ; i= 49  theta=1.20264  cos(theta)*32767=11792.681
  .byte hi8( 11039)  ; i= 50  theta=1.22718  cos(theta)*32767=11038.870
  .byte hi8( 10278)  ; i= 51  theta=1.25173  cos(theta)*32767=10278.410
  .byte hi8(  9512)  ; i= 52  theta=1.27627  cos(theta)*32767=9511.758
  .byte hi8(  8739)  ; i= 53  theta=1.30082  cos(theta)*32767=8739.377
  .byte hi8(  7962)  ; i= 54  theta=1.32536  cos(theta)*32767=7961.732
  .byte hi8(  7179)  ; i= 55  theta=1.34990  cos(theta)*32767=7179.290
  .byte hi8(  6393)  ; i= 56  theta=1.37445  cos(theta)*32767=6392.525
  .byte hi8(  5602)  ; i= 57  theta=1.39899  cos(theta)*32767=5601.908
  .byte hi8(  4808)  ; i= 58  theta=1.42353  cos(theta)*32767=4807.917
  .byte hi8(  4011)  ; i= 59  theta=1.44808  cos(theta)*32767=4011.031
  .byte hi8(  3212)  ; i= 60  theta=1.47262  cos(theta)*32767=3211.728
  .byte hi8(  2410)  ; i= 61  theta=1.49717  cos(theta)*32767=2410.490
  .byte hi8(  1608)  ; i= 62  theta=1.52171  cos(theta)*32767=1607.800
  .byte hi8(   804)  ; i= 63  theta=1.54625  cos(theta)*32767=804.142
  .byte hi8(     0)  ; i= 64  theta=1.57080  cos(theta)*32767=0.000
  .byte hi8(  -804)  ; i= 65  theta=1.59534  cos(theta)*32767=-804.142
  .byte hi8( -1608)  ; i= 66  theta=1.61988  cos(theta)*32767=-1607.800
  .byte hi8( -2410)  ; i= 67  theta=1.64443  cos(theta)*32767=-2410.490
  .byte hi8( -3212)  ; i= 68  theta=1.66897  cos(theta)*32767=-3211.728
  .byte hi8( -4011)  ; i= 69  theta=1.69351  cos(theta)*32767=-4011.031
  .byte hi8( -4808)  ; i= 70  theta=1.71806  cos(theta)*32767=-4807.917
  .byte hi8( -5602)  ; i= 71  theta=1.74260  cos(theta)*32767=-5601.908
  .byte hi8( -6393)  ; i= 72  theta=1.76715  cos(theta)*32767=-6392.525
  .byte hi8( -7179)  ; i= 73  theta=1.79169  cos(theta)*32767=-7179.290
  .byte hi8( -7962)  ; i= 74  theta=1.81623  cos(theta)*32767=-7961.732
  .byte hi8( -8739)  ; i= 75  theta=1.84078  cos(theta)*32767=-8739.377
  .byte hi8( -9512)  ; i= 76  theta=1.86532  cos(theta)*32767=-9511.758
  .byte hi8(-10278)  ; i= 77  theta=1.88986  cos(theta)*32767=-10278.410
  .byte hi8(-11039)  ; i= 78  theta=1.91441  cos(theta)*32767=-11038.870
  .byte hi8(-11793)  ; i= 79  theta=1.93895  cos(theta)*32767=-11792.681
  .byte hi8(-12539)  ; i= 80  theta=1.96350  cos(theta)*32767=-12539.388
  .byte hi8(-13279)  ; i= 81  theta=1.98804  cos(theta)*32767=-13278.542
  .byte hi8(-14010)  ; i= 82  theta=2.01258  cos(theta)*32767=-14009.698
  .byte hi8(-14732)  ; i= 83  theta=2.03713  cos(theta)*32767=-14732.414
  .byte hi8(-15446)  ; i= 84  theta=2.06167  cos(theta)*32767=-15446.257
  .byte hi8(-16151)  ; i= 85  theta=2.08621  cos(theta)*32767=-16150.795
  .byte hi8(-16846)  ; i= 86  theta=2.11076  cos(theta)*32767=-16845.605
  .byte hi8(-17530)  ; i= 87  theta=2.13530  cos(theta)*32767=-17530.267
  .byte hi8(-18204)  ; i= 88  theta=2.15984  cos(theta)*32767=-18204.370
  .byte hi8(-18868)  ; i= 89  theta=2.18439  cos(theta)*32767=-18867.507
  .byte hi8(-19519)  ; i= 90  theta=2.20893  cos(theta)*32767=-19519.279
  .byte hi8(-20159)  ; i= 91  theta=2.23348  cos(theta)*32767=-20159.294
  .byte hi8(-20787)  ; i= 92  theta=2.25802  cos(theta)*32767=-20787.165
  .byte hi8(-21403)  ; i= 93  theta=2.28256  cos(theta)*32767=-21402.515
  .byte hi8(-22005)  ; i= 94  theta=2.30711  cos(theta)*32767=-22004.972
  .byte hi8(-22594)  ; i= 95  theta=2.33165  cos(theta)*32767=-22594.175
  .byte hi8(-23170)  ; i= 96  theta=2.35619  cos(theta)*32767=-23169.768
  .byte hi8(-23731)  ; i= 97  theta=2.38074  cos(theta)*32767=-23731.404
  .byte hi8(-24279)  ; i= 98  theta=2.40528  cos(theta)*32767=-24278.746
  .byte hi8(-24811)  ; i= 99  theta=2.42983  cos(theta)*32767=-24811.462
  .byte hi8(-25329)  ; i=100  theta=2.45437  cos(theta)*32767=-25329.234
  .byte hi8(-25832)  ; i=101  theta=2.47891  cos(theta)*32767=-25831.747
  .byte hi8(-26319)  ; i=102  theta=2.50346  cos(theta)*32767=-26318.701
  .byte hi8(-26790)  ; i=103  theta=2.52800  cos(theta)*32767=-26789.802
  .byte hi8(-27245)  ; i=104  theta=2.55254  cos(theta)*32767=-27244.765
  .byte hi8(-27683)  ; i=105  theta=2.57709  cos(theta)*32767=-27683.317
  .byte hi8(-28105)  ; i=106  theta=2.60163  cos(theta)*32767=-28105.193
  .byte hi8(-28510)  ; i=107  theta=2.62618  cos(theta)*32767=-28510.140
  .byte hi8(-28898)  ; i=108  theta=2.65072  cos(theta)*32767=-28897.914
  .byte hi8(-29268)  ; i=109  theta=2.67526  cos(theta)*32767=-29268.281
  .byte hi8(-29621)  ; i=110  theta=2.69981  cos(theta)*32767=-29621.017
  .byte hi8(-29956)  ; i=111  theta=2.72435  cos(theta)*32767=-29955.911
  .byte hi8(-30273)  ; i=112  theta=2.74889  cos(theta)*32767=-30272.761
  .byte hi8(-30571)  ; i=113  theta=2.77344  cos(theta)*32767=-30571.375
  .byte hi8(-30852)  ; i=114  theta=2.79798  cos(theta)*32767=-30851.574
  .byte hi8(-31113)  ; i=115  theta=2.82252  cos(theta)*32767=-31113.190
  .byte hi8(-31356)  ; i=116  theta=2.84707  cos(theta)*32767=-31356.064
  .byte hi8(-31580)  ; i=117  theta=2.87161  cos(theta)*32767=-31580.050
  .byte hi8(-31785)  ; i=118  theta=2.89616  cos(theta)*32767=-31785.014
  .byte hi8(-31971)  ; i=119  theta=2.92070  cos(theta)*32767=-31970.832
  .byte hi8(-32137)  ; i=120  theta=2.94524  cos(theta)*32767=-32137.391
  .byte hi8(-32285)  ; i=121  theta=2.96979  cos(theta)*32767=-32284.593
  .byte hi8(-32412)  ; i=122  theta=2.99433  cos(theta)*32767=-32412.347
  .byte hi8(-32521)  ; i=123  theta=3.01887  cos(theta)*32767=-32520.577
  .byte hi8(-32609)  ; i=124  theta=3.04342  cos(theta)*32767=-32609.218
  .byte hi8(-32678)  ; i=125  theta=3.06796  cos(theta)*32767=-32678.216
  .byte hi8(-32728)  ; i=126  theta=3.09251  cos(theta)*32767=-32727.531
  .byte hi8(-32757)  ; i=127  theta=3.11705  cos(theta)*32767=-32757.131
  .byte hi8(-32767)  ; i=128  theta=3.14159  cos(theta)*32767=-32767.000
  .byte hi8(-32757)  ; i=129  theta=3.16614  cos(theta)*32767=-32757.131
  .byte hi8(-32728)  ; i=130  theta=3.19068  cos(theta)*32767=-32727.531
  .byte hi8(-32678)  ; i=131  theta=3.21522  cos(theta)*32767=-32678.216
  .byte hi8(-32609)  ; i=132  theta=3.23977  cos(theta)*32767=-32609.218
  .byte hi8(-32521)  ; i=133  theta=3.26431  cos(theta)*32767=-32520.577
  .byte hi8(-32412)  ; i=134  theta=3.28885  cos(theta)*32767=-32412.347
  .byte hi8(-32285)  ; i=135  theta=3.31340  cos(theta)*32767=-32284.593
  .byte hi8(-32137)  ; i=136  theta=3.33794  cos(theta)*32767=-32137.391
  .byte hi8(-31971)  ; i=137  theta=3.36249  cos(theta)*32767=-31970.832
  .byte hi8(-31785)  ; i=138  theta=3.38703  cos(theta)*32767=-31785.014
  .byte hi8(-31580)  ; i=139  theta=3.41157  cos(theta)*32767=-31580.050
  .byte hi8(-31356)  ; i=140  theta=3.43612  cos(theta)*32767=-31356.064
  .byte hi8(-31113)  ; i=141  theta=3.46066  cos(theta)*32767=-31113.190
  .byte hi8(-30852)  ; i=142  theta=3.48520  cos(theta)*32767=-30851.574
  .byte hi8(-30571)  ; i=143  theta=3.50975  cos(theta)*32767=-30571.375
  .byte hi8(-30273)  ; i=144  theta=3.53429  cos(theta)*32767=-30272.761
  .byte hi8(-29956)  ; i=145  theta=3.55884  cos(theta)*32767=-29955.911
  .byte hi8(-29621)  ; i=146  theta=3.58338  cos(theta)*32767=-29621.017
  .byte hi8(-29268)  ; i=147  theta=3.60792  cos(theta)*32767=-29268.281
  .byte hi8(-28898)  ; i=148  theta=3.63247  cos(theta)*32767=-28897.914
  .byte hi8(-28510)  ; i=149  theta=3.65701  cos(theta)*32767=-28510.140
  .byte hi8(-28105)  ; i=150  theta=3.68155  cos(theta)*32767=-28105.193
  .byte hi8(-27683)  ; i=151  theta=3.70610  cos(theta)*32767=-27683.317
  .byte hi8(-27245)  ; i=152  theta=3.73064  cos(theta)*32767=-27244.765
  .byte hi8(-26790)  ; i=153  theta=3.75518  cos(theta)*32767=-26789.802
  .byte hi8(-26319)  ; i=154  theta=3.77973  cos(theta)*32767=-26318.701
  .byte hi8(-25832)  ; i=155  theta=3.80427  cos(theta)*32767=-25831.747
  .byte hi8(-25329)  ; i=156  theta=3.82882  cos(theta)*32767=-25329.234
  .byte hi8(-24811)  ; i=157  theta=3.85336  cos(theta)*32767=-24811.462
  .byte hi8(-24279)  ; i=158  theta=3.87790  cos(theta)*32767=-24278.746
  .byte hi8(-23731)  ; i=159  theta=3.90245  cos(theta)*32767=-23731.404
  .byte hi8(-23170)  ; i=160  theta=3.92699  cos(theta)*32767=-23169.768
  .byte hi8(-22594)  ; i=161  theta=3.95153  cos(theta)*32767=-22594.175
  .byte hi8(-22005)  ; i=162  theta=3.97608  cos(theta)*32767=-22004.972
  .byte hi8(-21403)  ; i=163  theta=4.00062  cos(theta)*32767=-21402.515
  .byte hi8(-20787)  ; i=164  theta=4.02517  cos(theta)*32767=-20787.165
  .byte hi8(-20159)  ; i=165  theta=4.04971  cos(theta)*32767=-20159.294
  .byte hi8(-19519)  ; i=166  theta=4.07425  cos(theta)*32767=-19519.279
  .byte hi8(-18868)  ; i=167  theta=4.09880  cos(theta)*32767=-18867.507
  .byte hi8(-18204)  ; i=168  theta=4.12334  cos(theta)*32767=-18204.370
  .byte hi8(-17530)  ; i=169  theta=4.14788  cos(theta)*32767=-17530.267
  .byte hi8(-16846)  ; i=170  theta=4.17243  cos(theta)*32767=-16845.605
  .byte hi8(-16151)  ; i=171  theta=4.19697  cos(theta)*32767=-16150.795
  .byte hi8(-15446)  ; i=172  theta=4.22152  cos(theta)*32767=-15446.257
  .byte hi8(-14732)  ; i=173  theta=4.24606  cos(theta)*32767=-14732.414
  .byte hi8(-14010)  ; i=174  theta=4.27060  cos(theta)*32767=-14009.698
  .byte hi8(-13279)  ; i=175  theta=4.29515  cos(theta)*32767=-13278.542
  .byte hi8(-12539)  ; i=176  theta=4.31969  cos(theta)*32767=-12539.388
  .byte hi8(-11793)  ; i=177  theta=4.34423  cos(theta)*32767=-11792.681
  .byte hi8(-11039)  ; i=178  theta=4.36878  cos(theta)*32767=-11038.870
  .byte hi8(-10278)  ; i=179  theta=4.39332  cos(theta)*32767=-10278.410
  .byte hi8( -9512)  ; i=180  theta=4.41786  cos(theta)*32767=-9511.758
  .byte hi8( -8739)  ; i=181  theta=4.44241  cos(theta)*32767=-8739.377
  .byte hi8( -7962)  ; i=182  theta=4.46695  cos(theta)*32767=-7961.732
  .byte hi8( -7179)  ; i=183  theta=4.49150  cos(theta)*32767=-7179.290
  .byte hi8( -6393)  ; i=184  theta=4.51604  cos(theta)*32767=-6392.525
  .byte hi8( -5602)  ; i=185  theta=4.54058  cos(theta)*32767=-5601.908
  .byte hi8( -4808)  ; i=186  theta=4.56513  cos(theta)*32767=-4807.917
  .byte hi8( -4011)  ; i=187  theta=4.58967  cos(theta)*32767=-4011.031
  .byte hi8( -3212)  ; i=188  theta=4.61421  cos(theta)*32767=-3211.728
  .byte hi8( -2410)  ; i=189  theta=4.63876  cos(theta)*32767=-2410.490
  .byte hi8( -1608)  ; i=190  theta=4.66330  cos(theta)*32767=-1607.800
  .byte hi8(  -804)  ; i=191  theta=4.68785  cos(theta)*32767=-804.142
  .byte hi8(     0)  ; i=192  theta=4.71239  cos(theta)*32767=-0.000
  .byte hi8(   804)  ; i=193  theta=4.73693  cos(theta)*32767=804.142
  .byte hi8(  1608)  ; i=194  theta=4.76148  cos(theta)*32767=1607.800
  .byte hi8(  2410)  ; i=195  theta=4.78602  cos(theta)*32767=2410.490
  .byte hi8(  3212)  ; i=196  theta=4.81056  cos(theta)*32767=3211.728
  .byte hi8(  4011)  ; i=197  theta=4.83511  cos(theta)*32767=4011.031
  .byte hi8(  4808)  ; i=198  theta=4.85965  cos(theta)*32767=4807.917
  .byte hi8(  5602)  ; i=199  theta=4.88419  cos(theta)*32767=5601.908
  .byte hi8(  6393)  ; i=200  theta=4.90874  cos(theta)*32767=6392.525
  .byte hi8(  7179)  ; i=201  theta=4.93328  cos(theta)*32767=7179.290
  .byte hi8(  7962)  ; i=202  theta=4.95783  cos(theta)*32767=7961.732
  .byte hi8(  8739)  ; i=203  theta=4.98237  cos(theta)*32767=8739.377
  .byte hi8(  9512)  ; i=204  theta=5.00691  cos(theta)*32767=9511.758
  .byte hi8( 10278)  ; i=205  theta=5.03146  cos(theta)*32767=10278.410
  .byte hi8( 11039)  ; i=206  theta=5.05600  cos(theta)*32767=11038.870
  .byte hi8( 11793)  ; i=207  theta=5.08054  cos(theta)*32767=11792.681
  .byte hi8( 12539)  ; i=208  theta=5.10509  cos(theta)*32767=12539.388
  .byte hi8( 13279)  ; i=209  theta=5.12963  cos(theta)*32767=13278.542
  .byte hi8( 14010)  ; i=210  theta=5.15418  cos(theta)*32767=14009.698
  .byte hi8( 14732)  ; i=211  theta=5.17872  cos(theta)*32767=14732.414
  .byte hi8( 15446)  ; i=212  theta=5.20326  cos(theta)*32767=15446.257
  .byte hi8( 16151)  ; i=213  theta=5.22781  cos(theta)*32767=16150.795
  .byte hi8( 16846)  ; i=214  theta=5.25235  cos(theta)*32767=16845.605
  .byte hi8( 17530)  ; i=215  theta=5.27689  cos(theta)*32767=17530.267
  .byte hi8( 18204)  ; i=216  theta=5.30144  cos(theta)*32767=18204.370
  .byte hi8( 18868)  ; i=217  theta=5.32598  cos(theta)*32767=18867.507
  .byte hi8( 19519)  ; i=218  theta=5.35052  cos(theta)*32767=19519.279
  .byte hi8( 20159)  ; i=219  theta=5.37507  cos(theta)*32767=20159.294
  .byte hi8( 20787)  ; i=220  theta=5.39961  cos(theta)*32767=20787.165
  .byte hi8( 21403)  ; i=221  theta=5.42416  cos(theta)*32767=21402.515
  .byte hi8( 22005)  ; i=222  theta=5.44870  cos(theta)*32767=22004.972
  .byte hi8( 22594)  ; i=223  theta=5.47324  cos(theta)*32767=22594.175
  .byte hi8( 23170)  ; i=224  theta=5.49779  cos(theta)*32767=23169.768
  .byte hi8( 23731)  ; i=225  theta=5.52233  cos(theta)*32767=23731.404
  .byte hi8( 24279)  ; i=226  theta=5.54687  cos(theta)*32767=24278.746
  .byte hi8( 24811)  ; i=227  theta=5.57142  cos(theta)*32767=24811.462
  .byte hi8( 25329)  ; i=228  theta=5.59596  cos(theta)*32767=25329.234
  .byte hi8( 25832)  ; i=229  theta=5.62051  cos(theta)*32767=25831.747
  .byte hi8( 26319)  ; i=230  theta=5.64505  cos(theta)*32767=26318.701
  .byte hi8( 26790)  ; i=231  theta=5.66959  cos(theta)*32767=26789.802
  .byte hi8( 27245)  ; i=232  theta=5.69414  cos(theta)*32767=27244.765
  .byte hi8( 27683)  ; i=233  theta=5.71868  cos(theta)*32767=27683.317
  .byte hi8( 28105)  ; i=234  theta=5.74322  cos(theta)*32767=28105.193
  .byte hi8( 28510)  ; i=235  theta=5.76777  cos(theta)*32767=28510.140
  .byte hi8( 28898)  ; i=236  theta=5.79231  cos(theta)*32767=28897.914
  .byte hi8( 29268)  ; i=237  theta=5.81686  cos(theta)*32767=29268.281
  .byte hi8( 29621)  ; i=238  theta=5.84140  cos(theta)*32767=29621.017
  .byte hi8( 29956)  ; i=239  theta=5.86594  cos(theta)*32767=29955.911
  .byte hi8( 30273)  ; i=240  theta=5.89049  cos(theta)*32767=30272.761
  .byte hi8( 30571)  ; i=241  theta=5.91503  cos(theta)*32767=30571.375
  .byte hi8( 30852)  ; i=242  theta=5.93957  cos(theta)*32767=30851.574
  .byte hi8( 31113)  ; i=243  theta=5.96412  cos(theta)*32767=31113.190
  .byte hi8( 31356)  ; i=244  theta=5.98866  cos(theta)*32767=31356.064
  .byte hi8( 31580)  ; i=245  theta=6.01320  cos(theta)*32767=31580.050
  .byte hi8( 31785)  ; i=246  theta=6.03775  cos(theta)*32767=31785.014
  .byte hi8( 31971)  ; i=247  theta=6.06229  cos(theta)*32767=31970.832
  .byte hi8( 32137)  ; i=248  theta=6.08684  cos(theta)*32767=32137.391
  .byte hi8( 32285)  ; i=249  theta=6.11138  cos(theta)*32767=32284.593
  .byte hi8( 32412)  ; i=250  theta=6.13592  cos(theta)*32767=32412.347
  .byte hi8( 32521)  ; i=251  theta=6.16047  cos(theta)*32767=32520.577
  .byte hi8( 32609)  ; i=252  theta=6.18501  cos(theta)*32767=32609.218
  .byte hi8( 32678)  ; i=253  theta=6.20955  cos(theta)*32767=32678.216
  .byte hi8( 32728)  ; i=254  theta=6.23410  cos(theta)*32767=32727.531
  .byte hi8( 32757)  ; i=255  theta=6.25864  cos(theta)*32767=32757.131
