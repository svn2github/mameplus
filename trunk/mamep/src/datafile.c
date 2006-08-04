/****************************************************************************
 *      datafile.c
 *      History database engine
 *
 *      Token parsing by Neil Bradley
 *      Modifications and higher-level functions by John Butler
 ****************************************************************************/

static const char *stat_versions[] =
{
	"0.107u2    August     3rd 2006",
	"0.107u1    July      29th 2006",
	"0.107      July      23rd 2006",
	"0.106u13   July      20th 2006",
	"0.106u12   July      16th 2006",
	"0.106u11   July      10th 2006",
	"0.106u10   July       4th 2006",
	"0.106u9    June      28th 2006",
	"0.106u8    June      26th 2006",
	"0.106u7    June      22th 2006",
	"0.106u6    June      18th 2006",
	"0.106u5    June       8th 2006",
	"0.106u4    June       5th 2006",
	"0.106u3    June       1st 2006",
	"0.106u2    May       25th 2006",
	"0.106u1    May       18th 2006",
	"0.106      May       13th 2006",
	"0.105u5    May       11th 2006",
	"0.105u4    May        3rd 2006",
	"0.105u3    April     27th 2006",
	"0.105u2    April     19th 2006",
	"0.105u1    April     13th 2006",
	"0.105      April      4th 2006",
	"0.104u9    April      2nd 2006",
	"0.104u8    March     30th 2006",
	"0.104u7    March     23th 2006",
	"0.104u6    March     16th 2006",
	"0.104u5    March      9th 2006",
	"0.104u4    March      2nd 2006",
	"0.104u3    February  22nd 2006",
	"0.104u2    February  16th 2006",
	"0.104u1    February   9th 2006",
	"0.104      February   5th 2006",
	"0.103u5    February   2nd 2006",
	"0.103u4    January   26th 2006",
	"0.103u3    January   19th 2006",
	"0.103u2fix January   12th 2006",
	"0.103u2    January   10th 2006",
	"0.103u1    January    4th 2006",
	"0.103      December  30th 2005",
	"0.102u5    December  22th 2005",
	"0.102u4    December  15th 2005",
	"0.102u3    December   9th 2005",
	"0.102u2    December   1st 2005",
	"0.102u1    November  21st 2005",
	"0.102      November  13th 2005",
	"0.101u5    November   4th 2005",
	"0.101u4    October   30th 2005",
	"0.101u3    October   24th 2005",
	"0.101u2    October   17th 2005",
	"0.101u1    October   13th 2005",
	"0.101      October    8th 2005",
	"0.100u4    October    6th 2005",
	"0.100u3    September 29th 2005",
	"0.100u2    September 22nd 2005",
	"0.100u1    September 16th 2005",
	"0.100      September 14th 2005",
	"0.99u10    September 12th 2005",
	"0.99u9     September 10th 2005",
	"0.99u8     September  9th 2005",
	"0.99u7     September  5th 2005",
	"0.99u6     August    29th 2005",
	"0.99u5     August    29th 2005",
	"0.99u4     August    24th 2005",
	"0.99u3     August    22nd 2005",
	"0.99u2     August    14th 2005",
	"0.99u1     August    10th 2005",
	"0.99       August     6th 2005",
	"0.98u4     August     4th 2005",
	"0.98u3     July      31st 2005",
	"0.98u2     July      21st 2005",
	"0.98u1     July      16th 2005",
	"0.98       July      10th 2005",
	"0.97u7     July       7th 2005",
	"0.97u4     July       2nd 2005",
	"0.97u3     June      26th 2005",
	"0.97u2     June      19th 2005",
	"0.97u1     June       9th 2005",
	"0.97       June       2nd 2005",
	"0.96u4     May       30th 2005",
	"0.96u3     May       24th 2005",
	"0.96u2     May       16th 2005",
	"0.96u1     May        5th 2005",
	"0.96       May        3rd 2005",
	"0.95u6     April     25th 2005",
	"0.95u5     April     21st 2005",
	"0.95u4     April     21st 2005",
	"0.95u3     April     18th 2005",
	"0.95u2     April      5th 2005",
	"0.95u1     April      1st 2005",
	"0.95       March     27th 2005",
	"0.94u5     March     22nd 2005",
	"0.94u4     March     18th 2005",
	"0.94u3     March     15th 2005",
	"0.94u2     March     14th 2005",
	"0.94u1     March      7th 2005",
	"0.94       March      6th 2005",
	"0.93u3     March      4th 2005",
	"0.93u2     March      3rd 2005",
	"0.93u1     March      1st 2005",
	"0.93       February  27th 2005",
	"0.92u2000  February  15th 2005",
	"0.92       February  13th 2005",
	"0.91u2     February   3rd 2005",
	"0.91u1     January   30th 2005",
	"0.91       January   29th 2005",
	"0.90u4     January   23rd 2005",
	"0.90u3     January   17th 2005",
	"0.90u2     January   12th 2005",
	"0.90u1     January    8th 2005",
	"0.90       January    4th 2005",
	"0.89u6     December  17th 2004",
	"0.89u5     December  15th 2004",
	"0.89u4     December  11th 2004",
	"0.89u3     December   7th 2004",
	"0.89u2     December   3rd 2004",
	"0.89u1     November  28th 2004",
	"0.89       November  24th 2004",
	"0.88u7     November  20th 2004",
	"0.88u6     November  17th 2004",
	"0.88u5     November  15th 2004",
	"0.88u4     November  11th 2004",
	"0.88u3     November   7th 2004",
	"0.88u2     October   31st 2004",
	"0.88u1     October   28th 2004",
	"0.88       October   25th 2004",
	"0.87u4     October   16th 2004",
	"0.87u3     October   12th 2004",
	"0.87u2     October   10th 2004",
	"0.87u1     September 29th 2004",
	"0.87       September 21st 2004",
	"0.86u5     September 15th 2004",
	"0.86u4     September  9th 2004",
	"0.86u3     September  3rd 2004",
	"0.86u2     August    31st 2004",
	"0.86u1     August    27th 2004",
	"0.86       August    23rd 2004",
	"0.85u3     August    21st 2004",
	"0.85u2     August    13th 2004",
	"0.85u1     August    10th 2004",
	"0.85       August     7th 2004",
	"0.84u6     August     3rd 2004",
	"0.84u5     July      27th 2004",
	"0.84u4     July      25th 2004",
	"0.84u3     July      15th 2004",
	"0.84u2     July      11th 2004",
	"0.84u1     July       4th 2004",
	"0.84       July       2nd 2004",
	"0.83       June       5th 2004",
	"0.82u3     May       27th 2004",
	"0.82u2     May       23rd 2004",
	"0.82u1     May       13th 2004",
	"0.82       May        6th 2004",
	"0.81u9     April     28th 2004",
	"0.81u8     April     27th 2004",
	"0.81u7     April     22nd 2004",
	"0.81u6     April     17th 2004",
	"0.81u5     April     14th 2004",
	"0.81u4     April      8th 2004",
	"0.81u3     April      4th 2004",
	"0.81       April      1st 2004",
	"0.80u3     March     28th 2004",
	"0.80u2     March     27th 2004",
	"0.80u1     March     11th 2004",
	"0.80       March      6th 2004",
	"0.79u4     February  28th 2004",
	"0.79u3     February  25th 2004",
	"0.79u2     February  21st 2004",
	"0.79u1     February   8th 2004",
	"0.79       January   28th 2004",
	"0.78u6     January   20th 2004",
	"0.78u5     January   18th 2004",
	"0.78u4     January   13th 2004",
	"0.78u3     January   11th 2004",
	"0.78u2     January    9th 2004",
	"0.78u1     January    4th 2004",
	"0.78       December  25th 2003",
	"0.77u3     December  13th 2003",
	"0.77u2     December   1st 2003",
	"0.77u1     November  21st 2003",
	"0.77       November  12th 2003",
	"0.76u2     November   1st 2003",
	"0.76u1     October   25th 2003",
	"0.76       October   19th 2003",
	"0.75u1     October   14th 2003",
	"0.75       October   13th 2003",
	"0.74u2     October    6th 2003",
	"0.74u1     September 23rd 2003",
	"0.74       September 14th 2003",
	"0.73       September  4th 2003",
	"0.72u2     August    25th 2003",
	"0.72u1     August    15th 2003",
	"0.72       August     9th 2003",
	"0.71u2     July      12th 2003",
	"0.71u1     July       8th 2003",
	"0.71       July       5th 2003",
	"0.70u5     June      29th 2003",
	"0.70u4     June      21st 2003",
	"0.70u3     June      21st 2003",
	"0.70u2     June      19th 2003",
	"0.70u1     June      12th 2003",
	"0.70       June      11th 2003",
	"0.69u3     June       5th 2003",
	"0.69b      June       2nd 2003",
	"0.69a      May       31st 2003",
	"0.69       May       24th 2003",
	"0.68       May       16th 2003",
	"0.67       April      6th 2003",
	"0.66       March     10th 2003",
	"0.65       February  10th 2003",
	"0.64       January   26th 2003",
	"0.63       January   12th 2003",
	"0.62       November  12th 2002",
	"0.61       July       4th 2002",
	"0.60       May        1st 2002",
	"0.59       March     22nd 2002",
	"0.58       February   6th 2002",
	"0.57       January    1st 2002",
	"0.56       November   1st 2001",
	"0.55       September 17th 2001",
	"0.54       August    25th 2001",
	"0.53       August    12th 2001",
	"0.37b16    July       2nd 2001",
	"0.37b15    May       24th 2001",
	"0.37b14    April      7th 2001",
	"0.37b13    March     10th 2001",
	"0.37b12    February  15th 2001",
	"0.37b11    January   16th 2001",
	"0.37b10    December   6th 2000",
	"0.37b9     November   6th 2000",
	"0.37b8     October    3rd 2000",
	"0.37b7     September  5th 2000",
	"0.37b6     August    20th 2000",
	"0.37b5     July      28th 2000",
	"0.37b4     June      16th 2000",
	"0.37b3     May       27th 2000",
	"0.37b2     May        6th 2000",
	"0.37b1     April      6th 2000",
	"0.36       March     21st 2000",
	"0.36RC2    March     13th 2000",
	"0.36RC1    February  26th 2000",
	"0.36b16    February   4th 2000",
	"0.36b15    January   21st 2000",
	"0.36b14    January   11th 2000",
	"0.36b13    December  30th 1999",
	"0.36b12    December  18th 1999",
	"0.36b11    December   6th 1999",
	"0.36b10    November  20th 1999",
	"0.36b91    November  14th 1999",
	"0.36b9     November  13th 1999",
	"0.36b8     October   30th 1999",
	"0.36b7     October   17th 1999",
	"0.36b6     September 29th 1999",
	"0.36b5     September 18th 1999", 
	"0.36b4     September  4th 1999",
	"0.36b3     August    22nd 1999",
	"0.36b2     August     8th 1999",
	"0.36b1     July      19th 1999",
	"0.35fix    July       5th 1999",
	"0.35       July       4th 1999",
	"0.35RC2    June      24th 1999",
	"0.35RC1    June      13th 1999",
	"0.35b13    May       24th 1999",
	"0.35b12    May        1st 1999",
	"0.35b11    April     22nd 1999",
	"0.35b10    April      8th 1999",
	"0.35b9     March     30th 1999",
	"0.35b8     March     24th 1999",
	"0.35b7     March     18th 1999",
	"0.35b6     March     15th 1999",
	"0.35b5     March      7th 1999",
	"0.35b4     March      1st 1999",
	"0.35b3     February  15th 1999",
	"0.35b2     January   24th 1999",
	"0.35b1     January    7th 1999",
	"0.34       December  31st 1998",
	"0.34RC2    December  21st 1998",
	"0.34RC1    December  14th 1998",
	"0.34b8     November  29th 1998",
	"0.34b7     November  10th 1998",
	"0.34b6     October   28th 1998",
	"0.34b5     October   16th 1998",
	"0.34b4     October    4th 1998",
	"0.34b3     September 17th 1998",
	"0.34b2     August    30th 1998",
	"0.34b1     August    16th 1998",
	"0.33       August    10th 1998",
	"0.33RC1    July      29th 1998", 
	"0.33b7     July      21st 1998",
	"0.33b6     June      16th 1998",
	"0.33b5     June      10th 1998",
	"0.33b4     May       31st 1998",
	"0.33b3     May       17th 1998",
	"0.33b2     May        8th 1998",
	"0.33b1     May        3rd 1998",
	"0.31       April     25th 1998",
	"0.30       January    7th 1998",
	"0.29       October   20th 1997",
	"0.28       September  7th 1997",
	"0.27       August    10th 1997",
	"0.26a      July      18th 1997",
	"0.26       July      14th 1997",
	"0.25       June      28th 1997",
	"0.24       June      13th 1997",
	"0.23a      June       3rd 1997",
	"0.23       June       2nd 1997",
	"0.22       May       25th 1997",
	"0.21.5     May       16th 1997",
	"0.21       May       12th 1997",
	"0.20       May        5th 1997",
	"0.19       April     26th 1997",
	"0.18       April     20th 1997",
	"0.17       April     14th 1997",
	"0.16       April     13th 1997",
	"0.15       April      6th 1997",
	"0.14       April      2nd 1997",
	"0.13       March     30th 1997",
	"0.12       March     23rd 1997",
	"0.11       March     16th 1997",
	"0.10       March     13th 1997",
	"0.091      March      9th 1997",
	"0.09       March      9th 1997",
	"0.081      March      4th 1997",
	"0.08       March      4th 1997",
	"0.07       February  27th 1997",
	"0.06       February  23rd 1997",
	"0.05       February  20th 1997",
	"0.04       February  16th 1997",
	"0.03       February  13th 1997",
	"0.02       February   9th 1997",
	"0.01       February   5th 1997",
	0
};

static const char *stat_history[] =
{
	"0.107u2    873    6223  +3",
	"0.107u1    873    6220  +7",
	"0.107      872    6213  +18",
	"0.106u13   869    6195  +2",
	"0.106u12   869    6193  +0",
	"0.106u11   869    6193  +4",
	"0.106u10   868    6189  +2",
	"0.106u9    868    6187  +0",
	"0.106u8    868    6187  +0",
	"0.106u7    868    6187  +0",
	"0.106u6    868    6187  +12",
	"0.106u5    864    6175  +1",
	"0.106u4    864    6174  +0",
	"0.106u3    864    6174  +3",
	"0.106u2    864    6171  +5",
	"0.106u1    862    6166  +0",
	"0.106      862    6166  +0",
	"0.105u5    862    6166  +8",
	"0.105u4    860    6158  +8",
	"0.105u3    860    6150  +34",
	"0.105u2    865    6116  +13",
	"0.105u1    858    6103  +7",
	"0.105      858    6096  +0",
	"0.104u9    858    6096  +15",
	"0.104u8    856    6081  +14",
	"0.104u7    856    6067  +3",
	"0.104u6    856    6064  +4",
	"0.104u5    856    6060  +17",
	"0.104u4    856    6043  +9",
	"0.104u3    856    6034  +3",
	"0.104u2    856    6031  +2",
	"0.104u1    856    6029  +5",
	"0.104      856    6024  +8",
	"0.103u5    856    6016  +10",
	"0.103u4    856    6006  +6",
	"0.103u3    854    6000  +5",
	"0.103u2fix 854    5995  +2",
	"0.103u2    854    5993  +41",
	"0.103u1    851    5952  +9",
	"0.103      851    5943  +28",
	"0.102u5    849    5915  +22",
	"0.102u4    846    5893  +11",
	"0.102u3    846    5882  +11",
	"0.102u2    845    5871  +13",
	"0.102u1    845    5858  +10",
	"0.102      845    5848  +15",
	"0.101u5    844    5833  +5",
	"0.101u4    844    5828  +7",
	"0.101u3    841    5821  +4",
	"0.101u2    841    5817  +4",
	"0.101u1    839    5813  +3",
	"0.101      839    5810  +0",
	"0.100u4    839    5810  +7",
	"0.100u3    838    5803  +12",
	"0.100u2    836    5791  +8",
	"0.100u1    831    5783  +5",
	"0.100      830    5778  +2",
	"0.99u10    829    5776  +3",
	"0.99u9     829    5773  +2",
	"0.99u8     829    5771  +8",
	"0.99u7     829    5763  +7",
	"0.99u6     830    5756  +2",
	"0.99u5     830    5754  +2",
	"0.99u4     829    5752  +3",
	"0.99u3     829    5749  +6",
	"0.99u2     827    5743  +33",
	"0.99u1     813    5710  +5",
	"0.99       812    5705  +0",
	"0.98u4     812    5705  +4",
	"0.98u3     813    5701  +3",
	"0.98u2     812    5698  +6",
	"0.98u1     811    5692  +5",
	"0.98       810    5687  +0",
	"0.97u5     810    5687  +3",
	"0.97u4     811    5684  +5",
	"0.97u3     811    5679  +5",
	"0.97u2     811    5674  +5",
	"0.97u1     811    5669  +8",
	"0.97       811    5661  +0",
	"0.96u3     811    5660  +14",
	"0.96u2     809    5646  +10",
	"0.96u1     809    5636  +4",
	"0.96       809    5632  +19",
	"0.95u6     805    5613  +4",
	"0.95u5     804    5609  +0",
	"0.95u4     804    5609  +4",
	"0.95u3     804    5605  +10",
	"0.95u2     804    5595  +11",
	"0.95u1     800    5584  +6",
	"0.95       800    5578  +13",
	"0.94u5     800    5565  +6",
	"0.94u4     799    5559  +6",
	"0.94u3     799    5553  +9",
	"0.94u1     799    5544  +6",
	"0.94       797    5538  +1",
	"0.93u3     797    5537  +1",
	"0.93u2     797    5536  +10",
	"0.93u1     797    5525  +1",
	"0.93       797    5524  +12",
	"0.92u2000  796    5512  +4",
	"0.92       796    5508  +65",
	"0.91u2     790    5443  +4",
	"0.91u1     788    5439  +11",
	"0.91       786    5428  +9",
	"0.90u4     782    5419  +10",
	"0.90u3     782    5409  +11",
	"0.90u2     782    5398  +3",
	"0.90u1     782    5395  +2",
	"0.90       781    5393  +13",
	"0.89u6     781    5380  +17",
	"0.89u4     779    5363  +9",
	"0.89u3     779    5354  +13",
	"0.89u2     780    5341  +10",
	"0.89u1     780    5331  +18",
	"0.89       779    5313  +1",
	"0.88u7     779    5312  +12",
	"0.88u6     777    5300  +14",
	"0.88u4     776    5286  +11",
	"0.88u3     775    5275  +15",
	"0.88u2     774    5260  +7",
	"0.88u1     774    5253  +5",
	"0.88       775    5248  +21",
	"0.87u4     774    5227  +10",
	"0.87u3     770    5217  +18",
	"0.87u2     770    5199  +14",
	"0.87u1     770    5185  +40",
	"0.87       768    5145  +4",
	"0.86u5     764    5141  +3",
	"0.86u4     763    5138  +5",
	"0.86u3     761    5133  +2",
	"0.86u2     761    5131  +37",
	"0.86u1     761    5094  +6",
	"0.86       759    5088  +14",
	"0.85u3     753    5074  +3",
	"0.85u2     752    5071  +3",
	"0.85u1     752    5068  +7",
	"0.85       751    5061  +1",
	"0.84u6     751    5060  +6",
	"0.84u5     751    5054  +20",
	"0.84u3     749    5034  +13",
	"0.84u2     747    5021  +1",
	"0.84u1     747    5020  +6",
	"0.84       747    5014  +61",
	"0.83       742    4953  +11",
	"0.82u3     739    4942  +7",
	"0.82u2     738    4935  +7",
	"0.82u1     737    4928  +7",
	"0.82       738    4921  +15",
	"0.81u9     736    4906  +11",
	"0.81u7     732    4895  +4",
	"0.81u6     732    4891  +7",
	"0.81u5     731    4884  +7",
	"0.81u4     731    4877  +16",
	"0.81u3     728    4861  +20",
	"0.81       726    4841  +11",
	"0.80u3     724    4830  +5",
	"0.80u2     723    4825  +24",
	"0.80u1     714    4801  +5",
	"0.80       713    4796  +6",
	"0.79u4     712    4790  +22",
	"0.79u1     707    4768  +16",
	"0.79       709    4752  +21",
	"0.78u6     708    4731  +0",
	"0.78u5     708    4731  +4",
	"0.78u4     708    4727  +2",
	"0.78u3     711    4725  +7",
	"0.78u2     711    4718  +0",
	"0.78u1     711    4718  +13",
	"0.78       712    4705  +37",
	"0.77u3     707    4668  +8",
	"0.77u2     705    4660  +20",
	"0.77       702    4640  +32",
	"0.76u2     699    4608  +2",
	"0.76u1     698    4606  +107",
	"0.76       697    4499  +25",
	"0.75u1     696    4474  +1",
	"0.75       695    4473  +26",
	"0.74u2     693    4447  +313",
	"0.74u1     690    4134  +11",
	"0.74       686    4123  +7",
	"0.73       685    4116  +9",
	"0.72u2     683    4107  +19",
	"0.72u1     680    4088  +5",
	"0.72       679    4083  +31",
	"0.71u2     677    4052  +25",
	"0.71u1     669    4027  +3",
	"0.71       669    4024  +10",
	"0.70u5     667    4014  +8",
	"0.70u4     667    4006  +11",
	"0.70u3     666    3995  +2",
	"0.70u2     666    3993  +2",
	"0.70u1     665    3991  +1",
	"0.70       665    3990  +2",
	"0.69u3     664    3988  +10",
	"0.69b      664    3978  +7",
	"0.69a      663    3971  +7",
	"0.69       663    3964  +28",
	"0.68       657    3936  +110",
	"0.67       635    3826  +34",
	"0.66       631    3792  +43",
	"0.65       628    3749  +18",
	"0.64       626    3731  +41",
	"0.63       623    3690  +94",
	"0.62       605    3596  +127",
	"0.61       563    3469  +101",
	"0.60       543    3368  +78",
	"0.59       529    3290  +36",
	"0.58       521    3254  +25",
	"0.57       514    3229  +74",
	"0.56       502    3155  +32",
	"0.55       501    3123  +21",
	"0.54       499    3102  + 4",
	"0.53       499    3098  +90",
	"0.37b16    498    3008  +78",
	"0.37b15    478    2930  +47",
	"0.37b14    476    2883  +44",
	"0.37b13    471    2839  +66",
	"0.37b12    460    2773  +109",
	"0.37b11    448    2664  +125",
	"0.37b10    438    2539  +63",
	"0.37b9     421    2476  +65",
	"0.37b8     417    2411  +74",
	"0.37b7     407    2337  +54",
	"0.37b6     400    2283  +43",
	"0.37b5     393    2240  +83",
	"0.37b4     383    2157  +12",
	"0.37b3     382    2145  +42",
	"0.37b2     375    2103  +30",
	"0.37b1     369    2073  +25",
	"0.36       366    2048  + 0",
	"0.36RC2    366    2048  +27",
	"0.36RC1    364    2021  +21",
	"0.36b16    364    2000  +29",
	"0.36b15    366    1971  +20",
	"0.36b14    362    1951  +19",
	"0.36b13    361    1932  +20",
	"0.36b12    356    1912  +12",
	"0.36b11    353    1900  +23",
	"0.36b10    352    1877  +19",
	"0.36b91    345    1858  + 0",
	"0.36b9     345    1858  +27",
	"0.36b8     342    1831  +37",
	"0.36b7     339    1794  +29",
	"0.36b6     336    1765  +42",
	"0.36b5     330    1723  +39",
	"0.36b4     325    1684  +42",
	"0.36b3     320    1642  +41",
	"0.36b2     312    1601  +69",
	"0.36b1     297    1532  +58",
	"0.35fix    287    1474  + 0",
	"0.35       287    1474  +15",
	"0.35RC2    287    1459  +22",
	"0.35RC1    286    1437  +37",
	"0.35b13    285    1400  +58",
	"0.35b12    273    1342  +25",
	"0.35b11    268    1317  +54",
	"0.35b10    262    1263  + 9",
	"0.35b9     261    1254  +13",
	"0.35b8     258    1241  +22",
	"0.35b7     256    1219  + 2",
	"0.35b6     257    1217  +31",
	"0.35b5     255    1186  + 8",
	"0.35b4     255    1178  +30",
	"0.35b3     252    1148  +41",
	"0.35b2     251    1107  +60",
	"0.35b1     247    1047  +23",
	"0.34       243    1024  + 0",
	"0.34RC2    243    1024  + 0",
	"0.34RC1    243    1024  + 1",
	"0.34b8     244    1023  +43",
	"0.34b7     236     980  +42",
	"0.34b6     234     938  +34",
	"0.34b5     231     904  +50",
	"0.34b4     230     854  +50",
	"0.34b3     226     804  +38",
	"0.34b2     226     766  +64",
	"0.34b1     218     702  +73",
	"0.33       210     629  + 0",
	"0.33RC1    210     629  + 1",
	"0.33b7     210     628  +24",
	"0.33b6     200     604  +21",
	"0.33b5             583  +18",
	"0.33b4             565  +25",
	"0.33b3             540  +40",
	"0.33b2             500  + 7",
	"0.33b1             493  +15",
	"0.31               478  +132",
	"0.30               346  +88",
	"0.29               258  +33",
	"0.28               225  +25",
	"0.27               200  +32",
	"0.26a              168  + 1",
	"0.26               167  +21",
	"0.25               146  +12",
	"0.24               134  +10",
	"0.23a              124  + 0",
	"0.23               124  + 6",
	"0.22               118  + 6",
	"0.21.5             112  + 0",
	"0.21               112  + 6",
	"0.20               106  + 4",
	"0.19               102  + 4",
	"0.18                98  + 9",
	"0.17                89  + 4",
	"0.16                85  + 5",
	"0.15                80  + 2",
	"0.14                78  + 1",
	"0.13                77  + 1",
	"0.12                76  + 8",
	"0.11                68  + 3",
	"0.10                65  + 3",
	"0.091               62  + 0",
	"0.09                62  + 3",
	"0.081               59  + 0",
	"0.08                59  +11",
	"0.07                48  + 7",
	"0.06                41  + 7",
	"0.05                34  +15",
	"0.04                19  + 2",
	"0.03                17  + 1",
	"0.02                16  +11",
	"0.01                 5     ",
	0
};

static const char *stat_newgames[] =
{
	"1997:   34      +  258",
	"1998:   22      +  766",
	"1999:   31      +  908",
	"2000:   16      +  607",
	"2001:   10      +  616",
	"2002:    6      +  441",
	"2003:   36      + 1109",
	"2004:   65      +  675",
	"2005:   70      +  563",
	"2006:   22      +  280",
	0
};


/****************************************************************************
 *      include files
 ****************************************************************************/
#include <assert.h>
#include <ctype.h>
#include "osd_cpu.h"
#include "driver.h"
#include "datafile.h"
#include "hash.h"


/****************************************************************************
 *      token parsing constants
 ****************************************************************************/
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define CR	0x0d	/* '\n' and '\r' meanings are swapped in some */
#define LF	0x0a	/*     compilers (e.g., Mac compilers) */

enum
{
	TOKEN_COMMA,
	TOKEN_EQUALS,
	TOKEN_SYMBOL,
	TOKEN_LINEBREAK,
	TOKEN_INVALID=-1
};

#define MAX_TOKEN_LENGTH	1024


/****************************************************************************
 *      datafile constants
 ****************************************************************************/
#define MAX_MENUIDX_ENTRIES 64
#define DATAFILE_TAG '$'

const char *DATAFILE_TAG_KEY = "$info";
const char *DATAFILE_TAG_BIO = "$bio";
#ifdef STORY_DATAFILE
const char *DATAFILE_TAG_STORY = "$story";
#endif /* STORY_DATAFILE */
const char *DATAFILE_TAG_MAME = "$mame";
const char *DATAFILE_TAG_DRIV = "$drv";

#ifdef CMD_LIST
const char *DATAFILE_TAG_COMMAND = "$cmd";
const char *DATAFILE_TAG_END = "$end";
#endif /* CMD_LIST */

const char *history_filename = NULL;
#ifdef STORY_DATAFILE
const char *story_filename = NULL;
#endif /* STORY_DATAFILE */
const char *mameinfo_filename = NULL;

#ifdef CMD_LIST
const char *command_filename = NULL;
#endif /* CMD_LIST */

const char *lang_directory = NULL;

#define FILE_MERGED	1
#define FILE_ROOT	2
#define FILE_TYPEMAX	((FILE_MERGED | FILE_ROOT) + 1)

struct tDatafileIndex
{
	long offset;
	const game_driver *driver;
};

static struct tDatafileIndex *hist_idx[FILE_TYPEMAX];
#ifdef STORY_DATAFILE
static struct tDatafileIndex *story_idx[FILE_TYPEMAX];
#endif /* STORY_DATAFILE */
static struct tDatafileIndex *mame_idx[FILE_TYPEMAX];
static struct tDatafileIndex *driv_idx;

#ifdef CMD_LIST
struct tMenuIndex
{
	long offset;
	char *menuitem;
};

static struct tDatafileIndex *cmnd_idx[FILE_TYPEMAX];
static struct tMenuIndex *menu_idx;
static char *menu_filename;

static int mame32jp_wrap;
#endif /* CMD_LIST */

/****************************************************************************
 *      private data for parsing functions
 ****************************************************************************/
static mame_file *fp;				/* Our file pointer */
static long dwFilePos;				/* file position */
static UINT8 bToken[MAX_TOKEN_LENGTH];		/* Our current token */

/* an array of driver name/drivers array index sorted by driver name
   for fast look up by name */
typedef struct
{
	const char *name;
	int index;
} driver_data_type;
static driver_data_type *sorted_drivers = NULL;
static int num_games;


/**************************************************************************
 **************************************************************************
 *
 *      Parsing functions
 *
 **************************************************************************
 **************************************************************************/

/*
 * DriverDataCompareFunc -- compare function for GetGameNameIndex
 */
static int CLIB_DECL DriverDataCompareFunc(const void *arg1,const void *arg2)
{
	return strcmp( ((driver_data_type *)arg1)->name, ((driver_data_type *)arg2)->name );
}

/*
 * GetGameNameIndex -- given a driver name (in lowercase), return
 * its index in the main drivers[] array, or -1 if it's not found.
 */
static int GetGameNameIndex(const char *name)
{
	driver_data_type *driver_index_info;
	driver_data_type key;
	key.name = name;

	if (sorted_drivers == NULL)
	{
		/* initialize array of game names/indices */
		int i;

		sorted_drivers = (driver_data_type *)malloc(sizeof(driver_data_type) * num_games);
		for (i=0;i<num_games;i++)
		{
			sorted_drivers[i].name = drivers[i]->name;
			sorted_drivers[i].index = i;
		}
		qsort(sorted_drivers,num_games,sizeof(driver_data_type),DriverDataCompareFunc);
	}

	/* uses our sorted array of driver names to get the index in log time */
	driver_index_info = bsearch(&key,sorted_drivers,num_games,sizeof(driver_data_type),
	                            DriverDataCompareFunc);

	if (driver_index_info == NULL)
		return -1;

	return driver_index_info->index;

}


/****************************************************************************
 *      Create an array with sorted sourcedrivers for the function
 *      index_datafile_drivinfo to speed up the datafile access
 ****************************************************************************/

typedef struct
{
	const char *srcdriver;
	int index;
} srcdriver_data_type;
static srcdriver_data_type *sorted_srcdrivers = NULL;
static int num_games;


static int SrcDriverDataCompareFunc(const void *arg1,const void *arg2)
{
	return strcmp( ((srcdriver_data_type *)arg1)->srcdriver, ((srcdriver_data_type *)arg2)->srcdriver );
}


static int GetSrcDriverIndex(const char *srcdriver)
{
	srcdriver_data_type *srcdriver_index_info;
	srcdriver_data_type key;
	key.srcdriver = srcdriver;

	if (sorted_srcdrivers == NULL)
	{
		/* initialize array of game names/indices */
		int i;
		num_games = 0;
		while (drivers[num_games] != NULL)
			num_games++;

		sorted_srcdrivers = (srcdriver_data_type *)malloc(sizeof(srcdriver_data_type) * num_games);
		for (i=0;i<num_games;i++)
		{
			sorted_srcdrivers[i].srcdriver = drivers[i]->source_file+12;
			sorted_srcdrivers[i].index = i;
		}
		qsort(sorted_srcdrivers,num_games,sizeof(srcdriver_data_type),SrcDriverDataCompareFunc);
	}

	srcdriver_index_info = bsearch(&key,sorted_srcdrivers,num_games,sizeof(srcdriver_data_type),
	                               SrcDriverDataCompareFunc);

	if (srcdriver_index_info == NULL)
		return -1;

	return srcdriver_index_info->index;

}


/****************************************************************************
 *      GetNextToken - Pointer to the token string pointer
 *                     Pointer to position within file
 *
 *      Returns token, or TOKEN_INVALID if at end of file
 ****************************************************************************/
static UINT32 GetNextToken(UINT8 **ppszTokenText, long *pdwPosition)
{
	UINT32 dwLength;			/* Length of symbol */
	long dwPos;				/* Temporary position */
	UINT8 *pbTokenPtr = bToken;		/* Point to the beginning */
	UINT8 bData;				/* Temporary data byte */
#ifdef CMD_LIST
	UINT8 space, character;

	if (mame32jp_wrap)
	{
		space     = '\t';
		character = ' ' - 1;
	}
	else
	{
		space     = ' ';
		character = ' ';
	}
#endif /* CMD_LIST */

	while (1)
	{
		bData = mame_fgetc(fp);		/* Get next character */

		/* If we're at the end of the file, bail out */

		if (mame_feof(fp))
			return(TOKEN_INVALID);

		/* If it's not whitespace, then let's start eating characters */

#ifdef CMD_LIST
		if (space != bData && '\t' != bData)
#else /* CMD_LIST */
		if (' ' != bData && '\t' != bData)
#endif /* CMD_LIST */
		{
			/* Store away our file position (if given on input) */

			if (pdwPosition)
				*pdwPosition = dwFilePos;

			/* If it's a separator, special case it */

			if (',' == bData || '=' == bData)
			{
				*pbTokenPtr++ = bData;
				*pbTokenPtr = '\0';
				++dwFilePos;

				if (',' == bData)
					return(TOKEN_COMMA);
				else
					return(TOKEN_EQUALS);
			}

			/* Otherwise, let's try for a symbol */

#ifdef CMD_LIST
			if (bData > character)
#else /* CMD_LIST */
			if (bData > ' ')
#endif /* CMD_LIST */
			{
				dwLength = 0;			/* Assume we're 0 length to start with */

				/* Loop until we've hit something we don't understand */

				while (bData != ',' &&
				       bData != '=' &&
#ifdef CMD_LIST
				       bData != space &&
#else /* CMD_LIST */
				       bData != ' ' &&
#endif /* CMD_LIST */
				       bData != '\t' &&
				       bData != '\n' &&
				       bData != '\r' &&
				       mame_feof(fp) == 0)
				{
					++dwFilePos;
					*pbTokenPtr++ = bData;	/* Store our byte */
					++dwLength;
					assert(dwLength < MAX_TOKEN_LENGTH);
					bData = mame_fgetc(fp);
				}

				/* If it's not the end of the file, put the last received byte */
				/* back. We don't want to touch the file position, though if */
				/* we're past the end of the file. Otherwise, adjust it. */

				if (0 == mame_feof(fp))
				{
					mame_ungetc(bData, fp);
				}

				/* Null terminate the token */

				*pbTokenPtr = '\0';

				/* Connect up the */

				if (ppszTokenText)
					*ppszTokenText = bToken;

				return(TOKEN_SYMBOL);
			}

			/* Not a symbol. Let's see if it's a cr/cr, lf/lf, or cr/lf/cr/lf */
			/* sequence */

			if (LF == bData)
			{
				/* Unix style perhaps? */

				bData = mame_fgetc(fp);		/* Peek ahead */
				mame_ungetc(bData, fp);		/* Force a retrigger if subsequent LF's */

				if (LF == bData)		/* Two LF's in a row - it's a UNIX hard CR */
				{
					++dwFilePos;
					*pbTokenPtr++ = bData;	/* A real linefeed */
					*pbTokenPtr = '\0';

					return(TOKEN_LINEBREAK);
				}

				/* Otherwise, fall through and keep parsing. */

			}
			else if (CR == bData)			/* Carriage return? */
			{
				/* Figure out if it's Mac or MSDOS format */

				++dwFilePos;
				bData = mame_fgetc(fp);		/* Peek ahead */

				/* We don't need to bother with EOF checking. It will be 0xff if */
				/* it's the end of the file and will be caught by the outer loop. */

				if (CR == bData)		/* Mac style hard return! */
				{
					/* Do not advance the file pointer in case there are successive */
					/* CR/CR sequences */

					/* Stuff our character back upstream for successive CR's */

					mame_ungetc(bData, fp);

					*pbTokenPtr++ = bData;	/* A real carriage return (hard) */
					*pbTokenPtr = '\0';

					return(TOKEN_LINEBREAK);
				}
				else if (LF == bData)		/* MSDOS format! */
				{
#ifdef CMD_LIST
					if (mame32jp_wrap)
					{
						mame_ungetc(bData, fp);

						*pbTokenPtr++ = bData;	/* A real carriage return (hard) */
						*pbTokenPtr = '\0';

						return(TOKEN_LINEBREAK);
					}
					else
					{
#endif /* CMD_LIST */
						++dwFilePos;		/* Our file position to reset to */
						dwPos = dwFilePos;	/* Here so we can reposition things */

						/* Look for a followup CR/LF */

						bData = mame_fgetc(fp);	/* Get the next byte */

						if (CR == bData)	/* CR! Good! */
						{
							bData = mame_fgetc(fp);	/* Get the next byte */

							/* We need to do this to pick up subsequent CR/LF sequences */

							mame_fseek(fp, dwPos, SEEK_SET);

							if (pdwPosition)
								*pdwPosition = dwPos;

							if (LF == bData) 	/* LF? Good! */
							{
								*pbTokenPtr++ = '\r';
								*pbTokenPtr++ = '\n';
								*pbTokenPtr = '\0';

								return(TOKEN_LINEBREAK);
							}
						}
						else
						{
					 	       --dwFilePos;
							mame_ungetc(bData, fp);	/* Put the character back. No good */
						}
					}
#ifdef CMD_LIST
				}
#endif /* CMD_LIST */
				else
				{
					--dwFilePos;
					mame_ungetc(bData, fp);	/* Put the character back. No good */
				}

				/* Otherwise, fall through and keep parsing */
			}
		}

		++dwFilePos;
	}
}


#ifdef CMD_LIST
/****************************************************************************
 *      GetNextToken_ex - Pointer to the token string pointer
 *                        Pointer to position within file
 *
 *      Returns token, or TOKEN_INVALID if at end of file
 ****************************************************************************/
static UINT32 GetNextToken_ex(UINT8 **ppszTokenText, long *pdwPosition)
{
	UINT32 dwLength;			/* Length of symbol */
	long dwPos;				/* Temporary position */
	UINT8 *pbTokenPtr = bToken;		/* Point to the beginning */
	UINT8 bData;				/* Temporary data byte */

	while (1)
	{
		bData = mame_fgetc(fp);		/* Get next character */

		/* If we're at the end of the file, bail out */

		if (mame_feof(fp))
			return(TOKEN_INVALID);

		/* If it's not whitespace, then let's start eating characters */

		if ('\t' != bData)
		{
			/* Store away our file position (if given on input) */

			if (pdwPosition)
				*pdwPosition = dwFilePos;

			/* Fixed: exclude a separator of comma */
			/* If it's a separator, special case it */
//			if (',' == bData || '=' == bData)
			if ('=' == bData)
			{
				*pbTokenPtr++ = bData;
				*pbTokenPtr = '\0';
				++dwFilePos;

				return(TOKEN_EQUALS);
			}

			/* Otherwise, let's try for a symbol */
			if (bData >= ' ')
			{
				dwLength = 0;			/* Assume we're 0 length to start with */

				/* Loop until we've hit something we don't understand */

				/* Fixed: exclude a separator of comma and equal */
				while (
//				      (bData != ',' &&
//				       bData != '=' &&
				       bData != '\t' &&
				       bData != '\n' &&
				       bData != '\r' &&
				       mame_feof(fp) == 0)
				{
					++dwFilePos;
					*pbTokenPtr++ = bData;	/* Store our byte */
					++dwLength;
					assert(dwLength < MAX_TOKEN_LENGTH);
					bData = mame_fgetc(fp);
				}

				/* If it's not the end of the file, put the last received byte */
				/* back. We don't want to touch the file position, though if */
				/* we're past the end of the file. Otherwise, adjust it. */

				if (0 == mame_feof(fp))
				{
					mame_ungetc(bData, fp);
				}

				/* Null terminate the token */

				*pbTokenPtr = '\0';

				/* Connect up the */

				if (ppszTokenText)
					*ppszTokenText = bToken;

				return(TOKEN_SYMBOL);
			}

			/* Not a symbol. Let's see if it's a cr/cr, lf/lf, or cr/lf/cr/lf */
			/* sequence */

			if (LF == bData)
			{
				/* Unix style perhaps? */

				bData = mame_fgetc(fp);		/* Peek ahead */
				mame_ungetc(bData, fp);		/* Force a retrigger if subsequent LF's */

				if (LF == bData)		/* Two LF's in a row - it's a UNIX hard CR */
				{
					++dwFilePos;
					*pbTokenPtr++ = bData;	/* A real linefeed */
					*pbTokenPtr = '\0';

					return(TOKEN_LINEBREAK);
				}

				/* Otherwise, fall through and keep parsing. */

			}
			else if (CR == bData)			/* Carriage return? */
			{
				/* Figure out if it's Mac or MSDOS format */

				++dwFilePos;
				bData = mame_fgetc(fp);		/* Peek ahead */

				/* We don't need to bother with EOF checking. It will be 0xff if */
				/* it's the end of the file and will be caught by the outer loop. */

				if (CR == bData)		/* Mac style hard return! */
				{
					/* Do not advance the file pointer in case there are successive */
					/* CR/CR sequences */

					/* Stuff our character back upstream for successive CR's */

					mame_ungetc(bData, fp);

					*pbTokenPtr++ = bData;	/* A real carriage return (hard) */
					*pbTokenPtr = '\0';

					return(TOKEN_LINEBREAK);
				}
				else if (LF == bData)		/* MSDOS format! */
				{
					if (mame32jp_wrap)
					{
						mame_ungetc(bData, fp);

						*pbTokenPtr++ = bData;	/* A real carriage return (hard) */
						*pbTokenPtr = '\0';

						return(TOKEN_LINEBREAK);
					}
					else
					{
						++dwFilePos;		/* Our file position to reset to */
						dwPos = dwFilePos;	/* Here so we can reposition things */

						/* Look for a followup CR/LF */

						bData = mame_fgetc(fp);	/* Get the next byte */

						if (CR == bData)	/* CR! Good! */
						{
							bData = mame_fgetc(fp);	/* Get the next byte */

							/* We need to do this to pick up subsequent CR/LF sequences */

							mame_fseek(fp, dwPos, SEEK_SET);

							if (pdwPosition)
								*pdwPosition = dwPos;

							if (LF == bData) 	/* LF? Good! */
							{
								*pbTokenPtr++ = '\r';
								*pbTokenPtr++ = '\n';
								*pbTokenPtr = '\0';

								return(TOKEN_LINEBREAK);
							}
						}
						else
						{
								--dwFilePos;
							mame_ungetc(bData, fp);	/* Put the character back. No good */
						}
					}
				}
				else
				{
					--dwFilePos;
					mame_ungetc(bData, fp);	/* Put the character back. No good */
				}

				/* Otherwise, fall through and keep parsing */
			}
		}

		++dwFilePos;
	}
}
#endif /* CMD_LIST */


/****************************************************************************
 *      ParseClose - Closes the existing opened file (if any)
 ****************************************************************************/
static void ParseClose(void)
{
	/* If the file is open, time for fclose. */

	if (fp)
	{
		mame_fclose(fp);
	}

	fp = NULL;
}


/****************************************************************************
 *      ParseOpen - Open up file for reading
 ****************************************************************************/
static UINT8 ParseOpen(const char *pszFilename)
{
	/* Open file up in binary mode */

	fp = mame_fopen (NULL, pszFilename, FILETYPE_HISTORY, 0);

	/* If this is NULL, return FALSE. We can't open it */

	if (NULL == fp)
	{
		return(FALSE);
	}

	/* Otherwise, prepare! */

	dwFilePos = 0;
	return(TRUE);
}


/****************************************************************************
 *      ParseSeek - Move the file position indicator
 ****************************************************************************/
static UINT8 ParseSeek(long offset, int whence)
{
	int result = mame_fseek(fp, offset, whence);

	if (0 == result)
	{
		dwFilePos = mame_ftell(fp);
	}
	return (UINT8)result;
}


/**************************************************************************
 *      index_datafile
 *      Create an index for the records in the currently open datafile.
 *
 *      Returns 0 on error, or the number of index entries created.
 **************************************************************************/
static int index_datafile (struct tDatafileIndex **_index)
{
	struct tDatafileIndex *idx;
	int count = 0;
	UINT32 token = TOKEN_SYMBOL;

	num_games = 0;
	while (drivers[num_games] != NULL)
		num_games++;

	/* rewind file */
	if (ParseSeek (0L, SEEK_SET)) return 0;

	/* allocate index */
        idx = *_index = malloc ((num_games + 1) * sizeof (struct tDatafileIndex));
	if (NULL == idx) return 0;

	/* loop through datafile */
        while (count < num_games && TOKEN_INVALID != token)
	{
		long tell;
		UINT8 *s;

		token = GetNextToken (&s, &tell);
		if (TOKEN_SYMBOL != token) continue;

		/* DATAFILE_TAG_KEY identifies the driver */
		if (!mame_strnicmp (DATAFILE_TAG_KEY, (char *)s, strlen (DATAFILE_TAG_KEY)))
		{
			token = GetNextToken (&s, &tell);
			if (TOKEN_EQUALS == token)
			{
				int done = 0;

				token = GetNextToken (&s, &tell);
				while (count < num_games && !done && TOKEN_SYMBOL == token)
				{
					int game_index;
					UINT8 *p;
					for (p = s; *p; p++)
						*p = tolower(*p);

					game_index = GetGameNameIndex((char *)s);
					if (game_index >= 0)
					{
						idx->driver = drivers[game_index];
						idx->offset = tell;
						idx++;
						count++;
						/* done = 1;  Not done, as we must process other clones in list */
						//break;
					}

					if (!done)
					{
						token = GetNextToken (&s, &tell);

						if (TOKEN_COMMA == token)
							token = GetNextToken (&s, &tell);
						else
							done = 1;	/* end of key field */
					}
				}
			}
		}
	}

	/* mark end of index */
	idx->offset = 0L;
	idx->driver = 0;
	return count;
}

#ifdef CMD_LIST
/**************************************************************************
 *	index_menuidx
 *	
 *
 *	Returns 0 on error, or the number of index entries created.
 **************************************************************************/
static int index_menuidx (const game_driver *drv, struct tDatafileIndex *d_idx, struct tMenuIndex **_index)
{
	struct tMenuIndex *m_idx;
	const game_driver *gdrv;
	struct tDatafileIndex *gd_idx;
	int m_count = 0;
	UINT32 token = TOKEN_SYMBOL;

	long tell;
	long cmdtag_offset = 0;
	UINT8 *s;

	mame32jp_wrap = 1;

	gdrv = drv;
	do
	{
		gd_idx = d_idx;

		/* find driver in datafile index */
		while (gd_idx->driver)
		{
			if (gd_idx->driver == gdrv) break;
			gd_idx++;
		}

		if (gd_idx->driver == gdrv) break;
		gdrv = driver_get_clone(gdrv);
	} while (!gd_idx->driver && gdrv);

	if (gdrv == 0) return 0;	/* driver not found in Data_file_index */

	/* seek to correct point in datafile */
	if (ParseSeek (gd_idx->offset, SEEK_SET)) return 0;

	/* allocate index */
	m_idx = *_index = malloc(MAX_MENUIDX_ENTRIES * sizeof (struct tMenuIndex));
	if (NULL == m_idx) return 0;

	/* loop through between $cmd= */
	token = GetNextToken (&s, &tell);
	while ((m_count < (MAX_MENUIDX_ENTRIES - 1)) && TOKEN_INVALID != token)
	{
		if (!mame_strnicmp (DATAFILE_TAG_KEY, (char *)s, strlen (DATAFILE_TAG_KEY)))
			break;

		/* DATAFILE_TAG_COMMAND identifies the driver */
		if (!mame_strnicmp (DATAFILE_TAG_COMMAND, (char *)s, strlen (DATAFILE_TAG_COMMAND)))
		{
			cmdtag_offset = tell;
			token = GetNextToken_ex (&s, &tell);

			if (token == TOKEN_EQUALS)
				token = GetNextToken_ex (&s, &tell);
			else
			{
				while (TOKEN_SYMBOL != token)
					token = GetNextToken_ex (&s, &tell);
			}

			m_idx->menuitem = malloc(strlen((char *)s)+1);
			strcpy(m_idx->menuitem, (char *)s);

			m_idx->offset = cmdtag_offset;

			m_idx++;
			m_count++;
		}
		token = GetNextToken (&s, &tell);
	}

	/* mark end of index */
	m_idx->offset = 0L;
	m_idx->menuitem = NULL;

	return m_count;
}

static void free_menuidx(struct tMenuIndex **_index)
{
	if (*_index)
	{
		struct tMenuIndex *m_idx = *_index;

		while(m_idx->menuitem != NULL)
		{
			free(m_idx->menuitem);
			m_idx++;
		}

		free(*_index);
		*_index = NULL;
	}
}
#endif /* CMD_LIST */

static int index_datafile_drivinfo (struct tDatafileIndex **_index)
{
	struct tDatafileIndex *idx;
	int count = 0;
	UINT32 token = TOKEN_SYMBOL;

	num_games = 0;
	while (drivers[num_games] != NULL)
		num_games++;

	/* rewind file */
	if (ParseSeek (0L, SEEK_SET)) return 0;

	/* allocate index */
	idx = *_index = malloc ((num_games + 1) * sizeof (struct tDatafileIndex));
	if (NULL == idx) return 0;

	/* loop through datafile */
	while (count < num_games && TOKEN_INVALID != token)
	{
		long tell;
		UINT8 *s;

		token = GetNextToken (&s, &tell);
		if (TOKEN_SYMBOL != token) continue;

		/* DATAFILE_TAG_KEY identifies the driver */
		if (!mame_strnicmp (DATAFILE_TAG_KEY, (char *)s, strlen (DATAFILE_TAG_KEY)))
		{
			token = GetNextToken (&s, &tell);
			if (TOKEN_EQUALS == token)
			{
				int done = 0;

				token = GetNextToken (&s, &tell);
				while (count < num_games && !done && TOKEN_SYMBOL == token)
				{
					int src_index;
					strlwr(s);
					src_index = GetSrcDriverIndex(s);
					if (src_index >= 0)
					{
						idx->driver = drivers[src_index];
						idx->offset = tell;
						idx++;
						count++;
						/* done = 1;  Not done, as we must process other clones in list */
						//break;
					}

					if (!done)
					{
						token = GetNextToken (&s, &tell);
						if (TOKEN_COMMA == token)
							token = GetNextToken (&s, &tell);
						else
							done = 1;	/* end of key field */
					}
				}
			}
		}
	}

	/* mark end of index */
	idx->offset = 0L;
	idx->driver = 0;
	return count;
}


/**************************************************************************
 *      load_datafile_text
 *
 *      Loads text field for a driver into the buffer specified. Specify the
 *      driver, a pointer to the buffer, the buffer size, the index created by
 *      index_datafile(), and the desired text field (e.g., DATAFILE_TAG_BIO).
 *
 *      Returns 0 if successful.
 **************************************************************************/
static int load_datafile_text (const game_driver *drv, char *buffer, int bufsize,
                               struct tDatafileIndex *idx, const char *tag)
{
	int     offset = 0;
	int found = 0;
	UINT32  token = TOKEN_SYMBOL;
	UINT32  prev_token = TOKEN_SYMBOL;
#ifdef CMD_LIST
	int first = 1;
#endif /* CMD_LIST */

	*buffer = '\0';

	/* find driver in datafile index */
	while (idx->driver)
	{

		if (idx->driver == drv) break;

		idx++;
	}
	if (idx->driver == 0) return 1;	/* driver not found in index */

	/* seek to correct point in datafile */
	if (ParseSeek (idx->offset, SEEK_SET)) return 1;

	/* read text until buffer is full or end of entry is encountered */
	while (TOKEN_INVALID != token)
	{
		UINT8 *s;
		int len;
		long tell;

		token = GetNextToken (&s, &tell);
		if (TOKEN_INVALID == token) continue;

		if (found)
		{
			/* end entry when a tag is encountered */
			if (TOKEN_SYMBOL == token && DATAFILE_TAG == s[0] && TOKEN_LINEBREAK == prev_token) break;

			prev_token = token;

			/* translate platform-specific linebreaks to '\n' */
			if (TOKEN_LINEBREAK == token)
				strcpy ((char *)s, "\n");

			/* append a space to words */
#ifdef CMD_LIST
			if (!mame32jp_wrap && TOKEN_LINEBREAK != token)
#else /* CMD_LIST */
			if (TOKEN_LINEBREAK != token)
#endif /* CMD_LIST */
				strcat ((char *)s, " ");

			/* remove extraneous space before commas */
			if (TOKEN_COMMA == token)
			{
				--buffer;
				--offset;
				*buffer = '\0';
			}

			/* Get length of text to add to the buffer */
			len = strlen ((char *)s);

			/* Check for buffer overflow */
			/* For some reason we can get a crash if we try */
			/* to use the last 30 characters of the buffer  */
			if ((bufsize - offset) - len <= 45)
			{
				strcpy ((char *)s, " ...[TRUNCATED]");
				len = strlen((char *)s);
				strcpy (buffer, (char *)s);
				buffer += len;
				offset += len;
				break;
			}

			/* add this word to the buffer */
			strcpy (buffer, (char *)s);
			buffer += len;
			offset += len;
		}
		else
		{
			if (TOKEN_SYMBOL == token)
			{
				/* looking for requested tag */
				if (!mame_strnicmp (tag, (char *)s, strlen (tag)))
				{
					found = 1;
#ifdef CMD_LIST
					if (first && mame32jp_wrap)
					{
						mame_fseek(fp, 2l, SEEK_CUR);
						first = 0;
					}
#endif /* CMD_LIST */
				}
				else if (!mame_strnicmp (DATAFILE_TAG_KEY, (char *)s, strlen (DATAFILE_TAG_KEY)))
					break;	/* error: tag missing */
			}
		}
	}
	return (!found);
}

#ifdef CMD_LIST
/**************************************************************************
 *	load_datafile_text_ex
 *
 *	Loads text field for a driver into the buffer specified. Specify the
 *	driver, a pointer to the buffer, the buffer size, the index created by
 *	index_datafile(), and the desired text field (e.g., DATAFILE_TAG_BIO).
 *
 *	Returns 0 if successful.
 **************************************************************************/
static int load_datafile_text_ex (char *buffer, int bufsize,
	const char *tag, struct tMenuIndex *m_idx, const int menu_sel)
{
	int offset = 0;
	UINT32	token = TOKEN_SYMBOL;
	UINT32 	prev_token = TOKEN_SYMBOL;
	UINT8 *s = NULL;
	int len;
	long tell;

	*buffer = '\0';

	/* seek to correct point in datafile */
	if (ParseSeek ((m_idx + menu_sel)->offset, SEEK_SET)) return 1;

	/* read text until tag is found */
	while (TOKEN_INVALID != token)
	{
		token = GetNextToken (&s, &tell);

		if (TOKEN_INVALID == token)
			break;

		if (TOKEN_SYMBOL == token)
		{
			/* looking for requested tag */
			if (!mame_strnicmp (tag, (char *)s, strlen (tag)))
			{
				token = GetNextToken_ex (&s, &tell);

				if (TOKEN_EQUALS == token)
					token = GetNextToken_ex (&s, &tell);
				else
				{
					while (TOKEN_SYMBOL != token)
						token = GetNextToken_ex (&s, &tell);
				}

				break;
			}
			else if (!mame_strnicmp (DATAFILE_TAG_KEY, (char *)s, strlen (DATAFILE_TAG_KEY)))
			{
				token = TOKEN_INVALID;
				break;	/* error: tag missing */
			}
		}
	}

	/* read text until buffer is full or end of entry is encountered */
	while (TOKEN_INVALID != token)
	{
		/* end entry when a tag is encountered */
		if (TOKEN_SYMBOL == token && !mame_strnicmp (DATAFILE_TAG_END, (char *)s, strlen (DATAFILE_TAG_END)))
			break;

		prev_token = token;

		/* translate platform-specific linebreaks to '\n' */
		if (TOKEN_LINEBREAK == token)
			strcpy ((char *)s, "\n");

		/* add this word to the buffer */
		len = strlen ((char *)s);
		if ((len + offset) >= bufsize) break;
		strcpy (buffer, (char *)s);
		buffer += len;
		offset += len;

		token = GetNextToken_ex (&s, &tell);
	}

	return TOKEN_INVALID == token;
}
#endif /* CMD_LIST */

static int load_drivfile_text (const game_driver *drv, char *buffer, int bufsize,
	struct tDatafileIndex *idx, const char *tag)
{
	int     offset = 0;
	int     found = 0;
	UINT32  token = TOKEN_SYMBOL;
	UINT32  prev_token = TOKEN_SYMBOL;
	struct tDatafileIndex *idx_save = idx;

	*buffer = '\0';

	/* find driver in datafile index */
	while (idx->driver)
	{
		if (idx->driver->source_file == drv->source_file) break;
		idx++;
	}
	// MSVC doesn't work above, retry but slow. :-(
	if (idx->driver == 0)
	{
		idx = idx_save;

		while (idx->driver)
		{
			if (!strcmp(idx->driver->source_file, drv->source_file))
				break;
			idx++;
		}
	}

	if (idx->driver == 0) return 1;	/* driver not found in index */

	/* seek to correct point in datafile */
	if (ParseSeek (idx->offset, SEEK_SET)) return 1;

	/* read text until buffer is full or end of entry is encountered */
	while (TOKEN_INVALID != token)
	{
		UINT8 *s;
		int len;
		long tell;

		token = GetNextToken (&s, &tell);
		if (TOKEN_INVALID == token) continue;

		if (found)
		{
			/* end entry when a tag is encountered */
			if (TOKEN_SYMBOL == token && DATAFILE_TAG == s[0] && TOKEN_LINEBREAK == prev_token) break;

			prev_token = token;

			/* translate platform-specific linebreaks to '\n' */
			if (TOKEN_LINEBREAK == token)
				strcpy ((char *)s, "\n");

			/* append a space to words */
			if (TOKEN_LINEBREAK != token)
				strcat ((char *)s, " ");

			/* remove extraneous space before commas */
			if (TOKEN_COMMA == token)
			{
				--buffer;
				--offset;
				*buffer = '\0';
			}

			/* add this word to the buffer */
			len = strlen ((char *)s);
			if ((len + offset) >= bufsize) break;
			strcpy (buffer, (char *)s);
			buffer += len;
			offset += len;
		}
		else
		{
			if (TOKEN_SYMBOL == token)
			{
				/* looking for requested tag */
				if (!mame_strnicmp (tag, (char *)s, strlen (tag)))
					found = 1;
				else if (!mame_strnicmp (DATAFILE_TAG_KEY, (char *)s, strlen (DATAFILE_TAG_KEY)))
					break;	/* error: tag missing */
			}
		}
	}
	return (!found);
}


/**************************************************************************
 *	load_datafile
 *
 *	Returns 0 if successful,
 *              1 if failure.
 *
 *	NOTE: For efficiency the indices are never freed (intentional leak).
 **************************************************************************/
static int load_datafile (const game_driver *drv, char *buffer, int bufsize,
                             const char *tag, int where, struct tDatafileIndex *idx[],
                             const char *separated_dir, const char *merged_name)
{
	const game_driver *gdrv;
	char filename[80];
	char *base;

	filename[0] = '\0';

	if (where != FILE_ROOT)
	{
		if (!lang_directory)
			lang_directory = "lang";

		sprintf(filename, "%s\\%s\\",
	        	lang_directory,
			ui_lang_info[options.langcode].name);
	}

	base = filename + strlen(filename);

	for (gdrv = drv; gdrv && gdrv->name[0]; gdrv = driver_get_clone(gdrv))
	{
		int i;

		for (i = where; i <= where + FILE_MERGED; i += FILE_MERGED)
		{
			int status = 0;
			int err;

			if (i & FILE_MERGED)
			{
				strcpy(base, merged_name);

				/* try to open datafile */
				if (!ParseOpen (filename))
					continue;

				/* create index if necessary */
				if (idx[i])
					status = 1;
				else
					status = (index_datafile (&idx[i]) != 0);

				/* load text */
				if (idx[i])
				{
					int len = strlen (buffer);

					err = load_datafile_text (gdrv, buffer+len, bufsize-len, idx[i], tag);
					if (err) status = 0;
				}
			}
			else
			{
				const game_driver *pdrv;

				strcpy(base, separated_dir);
				strcat(base, gdrv->name);
				strcat(base, ".dat");

				/* try to open datafile */
				if (!ParseOpen (filename))
					continue;

				/* create index */
				if (idx[i])
				{
					free(idx[i]);
					idx[i] = 0;
				}

				status = (index_datafile (&idx[i]) != 0);

				pdrv = drv;
				do
				{
					int len = strlen (buffer);

					err = load_datafile_text (pdrv, buffer+len, bufsize-len, idx[i], tag);

					if (pdrv == gdrv)
						break;

					pdrv = driver_get_clone(pdrv);
				} while (err && pdrv);

				if (err) status = 0;
			}

			ParseClose ();

			if (status)
				return 0;
		}
	}

	return 1;
}

/**************************************************************************
 *	flush_index_if_needed
 **************************************************************************/
static void flush_index_if_needed(void)
{
	static int oldLangCode = -1;

	if (oldLangCode != options.langcode)
	{
		int i;

		for (i = 0; i < FILE_TYPEMAX; i++)
		{
			if (i & FILE_ROOT)
				continue;

			if (hist_idx[i])
			{
				free(hist_idx[i]);
				hist_idx[i] = 0;
			}

#ifdef STORY_DATAFILE
			if (story_idx[i])
			{
				free(story_idx[i]);
				story_idx[i] = 0;
			}
#endif /* STORY_DATAFILE */

			if (mame_idx[i])
			{
				free(mame_idx[i]);
				mame_idx[i] = 0;
			}

#ifdef CMD_LIST
			if (cmnd_idx[i])
			{
				free_menuidx(&menu_idx);
				free(cmnd_idx[i]);
				cmnd_idx[i] = 0;
			}
#endif /* CMD_LIST */
		}

		oldLangCode = options.langcode;
	}
}

/**************************************************************************
 *	load_driver_history
 *	Load history text for the specified driver into the specified buffer.
 *	Combines $bio field of HISTORY.DAT with $mame field of MAMEINFO.DAT.
 *
 *	Returns 0 if successful.
 *
 *	NOTE: For efficiency the indices are never freed (intentional leak).
 **************************************************************************/
int load_driver_history (const game_driver *drv, char *buffer, int bufsize)
{
	int result = 1;

	*buffer = 0;

#ifdef CMD_LIST
	mame32jp_wrap = 0;
#endif /* CMD_LIST */

	flush_index_if_needed();

	if (!history_filename)
		history_filename = "history.dat";

	result &= load_datafile (drv, buffer, bufsize,
	                         DATAFILE_TAG_BIO, 0, hist_idx,
	                         "history/", "history.dat");
	result &= load_datafile (drv, buffer, bufsize,
	                         DATAFILE_TAG_BIO, FILE_ROOT, hist_idx,
	                         "history/", history_filename);

	return result;
}

#ifdef STORY_DATAFILE
int load_driver_story (const game_driver *drv, char *buffer, int bufsize)
{
	int result = 1;

	*buffer = 0;

#ifdef CMD_LIST
	mame32jp_wrap = 0;
#endif /* CMD_LIST */

	if (!story_filename)
		story_filename = "story.dat";

	{
		int skip_pos = strlen(buffer);
		int check_pos;

		strcpy(buffer + skip_pos, _("\nStory:\n"));
		check_pos = strlen(buffer);

		result &= load_datafile (drv, buffer, bufsize,
		                         DATAFILE_TAG_STORY, 0, story_idx,
		                         "story/", "story.dat");
		result &= load_datafile (drv, buffer, bufsize,
		                         DATAFILE_TAG_STORY, FILE_ROOT, story_idx,
		                         "story/", story_filename);

		if (buffer[check_pos] == '\0')
			buffer[skip_pos] = '\0';
	}

	return result;
}
#endif /* STORY_DATAFILE */

int load_driver_mameinfo (const game_driver *drv, char *buffer, int bufsize)
{
	const rom_entry *region, *rom, *chunk;
	const game_driver *clone_of;
	machine_config game;
	int result = 1;
	int i;

	*buffer = 0;

	strcat(buffer, "MAMEInfo:\n");
	expand_machine_driver(drv->drv, &game);

	/* List the game info 'flags' */
	if (drv->flags &
	    ( GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS |
	      GAME_IMPERFECT_COLORS | GAME_NO_SOUND | GAME_IMPERFECT_SOUND | GAME_NO_COCKTAIL))
	{
		strcat(buffer, _("GAME: "));
		strcat(buffer, options.use_lang_list?
			_LST(drv->description):
			drv->description);
		strcat(buffer, "\n");

		if (drv->flags & GAME_NOT_WORKING)
			strcat(buffer, _("THIS GAME DOESN'T WORK. You won't be able to make it work correctly.  Don't bother.\n"));

		if (drv->flags & GAME_UNEMULATED_PROTECTION)
			strcat(buffer, _("The game has protection which isn't fully emulated.\n"));

		if (drv->flags & GAME_IMPERFECT_GRAPHICS)
			strcat(buffer, _("The video emulation isn't 100% accurate.\n"));

		if (drv->flags & GAME_WRONG_COLORS)
			strcat(buffer, _("The colors are completely wrong.\n"));

		if (drv->flags & GAME_IMPERFECT_COLORS)
			strcat(buffer, _("The colors aren't 100% accurate.\n"));

		if (drv->flags & GAME_NO_SOUND)
			strcat(buffer, _("The game lacks sound.\n"));

		if (drv->flags & GAME_IMPERFECT_SOUND)
			strcat(buffer, _("The sound emulation isn't 100% accurate.\n"));

		if (drv->flags & GAME_NO_COCKTAIL)
			strcat(buffer, _("Screen flipping in cocktail mode is not supported.\n"));

		strcat(buffer, "\n");
	}	

#ifdef CMD_LIST
	mame32jp_wrap = 0;
#endif /* CMD_LIST */

	flush_index_if_needed();

	if (!mameinfo_filename)
		mameinfo_filename = "mameinfo.dat";

	result &= load_datafile (drv, buffer, bufsize,
	                         DATAFILE_TAG_MAME, 0, mame_idx,
	                         "mameinfo/", "mameinfo.dat");
	result &= load_datafile (drv, buffer, bufsize,
	                         DATAFILE_TAG_MAME, FILE_ROOT, mame_idx,
	                         "mameinfo/", mameinfo_filename);

	strcat(buffer, _("\nROM REGION:\n"));
	for (region = rom_first_region(drv); region; region = rom_next_region(region))
		for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
		{
			char name[100];
			int length;

			length = 0;
			for (chunk = rom_first_chunk(rom); chunk; chunk = rom_next_chunk(chunk))
				length += ROM_GETLENGTH(chunk);

			sprintf(name," %-12s ",ROM_GETNAME(rom));
			strcat(buffer, name);
			sprintf(name,"%6x ",length);
			strcat(buffer, name);
			switch (ROMREGION_GETTYPE(region))
			{
			case REGION_CPU1: strcat(buffer, "cpu1"); break;
			case REGION_CPU2: strcat(buffer, "cpu2"); break;
			case REGION_CPU3: strcat(buffer, "cpu3"); break;
			case REGION_CPU4: strcat(buffer, "cpu4"); break;
			case REGION_CPU5: strcat(buffer, "cpu5"); break;
			case REGION_CPU6: strcat(buffer, "cpu6"); break;
			case REGION_CPU7: strcat(buffer, "cpu7"); break;
			case REGION_CPU8: strcat(buffer, "cpu8"); break;
			case REGION_GFX1: strcat(buffer, "gfx1"); break;
			case REGION_GFX2: strcat(buffer, "gfx2"); break;
			case REGION_GFX3: strcat(buffer, "gfx3"); break;
			case REGION_GFX4: strcat(buffer, "gfx4"); break;
			case REGION_GFX5: strcat(buffer, "gfx5"); break;
			case REGION_GFX6: strcat(buffer, "gfx6"); break;
			case REGION_GFX7: strcat(buffer, "gfx7"); break;
			case REGION_GFX8: strcat(buffer, "gfx8"); break;
			case REGION_PROMS: strcat(buffer, "prom"); break;
			case REGION_SOUND1: strcat(buffer, "snd1"); break;
			case REGION_SOUND2: strcat(buffer, "snd2"); break;
			case REGION_SOUND3: strcat(buffer, "snd3"); break;
			case REGION_SOUND4: strcat(buffer, "snd4"); break;
			case REGION_SOUND5: strcat(buffer, "snd5"); break;
			case REGION_SOUND6: strcat(buffer, "snd6"); break;
			case REGION_SOUND7: strcat(buffer, "snd7"); break;
			case REGION_SOUND8: strcat(buffer, "snd8"); break;
			case REGION_USER1: strcat(buffer, "usr1"); break;
			case REGION_USER2: strcat(buffer, "usr2"); break;
			case REGION_USER3: strcat(buffer, "usr3"); break;
			case REGION_USER4: strcat(buffer, "usr4"); break;
			case REGION_USER5: strcat(buffer, "usr5"); break;
			case REGION_USER6: strcat(buffer, "usr6"); break;
			case REGION_USER7: strcat(buffer, "usr7"); break;
			case REGION_USER8: strcat(buffer, "usr8"); break;
			}

		sprintf(name," %7x\n",ROM_GETOFFSET(rom));
		strcat(buffer, name);

		}

	clone_of = driver_get_clone(drv);
	if (clone_of && !(clone_of->flags & NOT_A_DRIVER))
	{
		strcat(buffer, _("\nORIGINAL:\n"));
		strcat(buffer, options.use_lang_list?
			_LST(clone_of->description):
			clone_of->description);
		strcat(buffer, _("\n\nCLONES:\n"));
		for (i = 0; drivers[i]; i++)
		{
			if (!mame_stricmp (drv->parent, drivers[i]->parent)) 
			{
				strcat(buffer, options.use_lang_list?
					_LST(drivers[i]->description):
					drivers[i]->description);
				strcat(buffer, "\n");
			}
		}
	}
	else
	{
		strcat(buffer, _("\nORIGINAL:\n"));
		strcat(buffer, options.use_lang_list?
			_LST(drv->description):
			drv->description);
		strcat(buffer, _("\n\nCLONES:\n"));
		for (i = 0; drivers[i]; i++)
		{
			if (!mame_stricmp (drv->name, drivers[i]->parent)) 
			{
				strcat(buffer, options.use_lang_list?
					_LST(drivers[i]->description):
					drivers[i]->description);
				strcat(buffer, "\n");
			}
		}
	}

	return 0;
}

int load_driver_drivinfo (const game_driver *drv, char *buffer, int bufsize)
{
	int drivinfo = 0;
	// int i;

	*buffer = 0;

	/* Print source code file */
	sprintf (buffer, _("\nSOURCE: %s\n"), drv->source_file+12);

	if (!mameinfo_filename)
		mameinfo_filename = "mameinfo.dat";

	/* Try to open mameinfo datafile - driver section */
	if (ParseOpen (mameinfo_filename))
	{
		int err;

		/* create index if necessary */
		if (driv_idx)
			drivinfo = 1;
		else
			drivinfo = (index_datafile_drivinfo (&driv_idx) != 0);

		/* load informational text (append) */
		if (driv_idx)
		{
			int len = strlen (buffer);

			err = load_drivfile_text (drv, buffer+len, bufsize-len,
			                          driv_idx, DATAFILE_TAG_DRIV);

			if (err) drivinfo = 0;
		}
		ParseClose ();
	}

/* Redundant Info
	strcat(buffer,"\nGAMES SUPPORTED:\n");
	for (i = 0; drivers[i]; i++)
	{
		if (!mame_stricmp (drv->source_file, drivers[i]->source_file)) 
		{
			strcat(buffer, options.use_lang_list?
				_LST(drivers[i]->description)):
				drivers[i]->description);
			strcat(buffer,"\n");
		}
	}
*/

	return (drivinfo == 0);
}

int load_driver_statistics (char *buffer, int bufsize)
{
	const rom_entry *region, *rom, *chunk;
	const rom_entry *pregion, *prom, *fprom=NULL;
	const game_driver *clone_of = NULL;
	const input_port_entry *inp;

	char name[100];
	char year[4];
	int i, n, x, y;
	int all = 0, cl = 0, vec = 0, vecc = 0, neo = 0, neoc = 0;
	int pch = 0, pchc = 0, deco = 0, decoc = 0, cvs = 0, cvsc = 0, noyear = 0, nobutt = 0, noinput = 0;
	int vertical = 0, verticalc = 0, horizontal = 0, horizontalc = 0;
	int clone = 0, stereo = 0, stereoc = 0;
	int sum = 0, xsum = 0, files = 0, mfiles = 0, hddisk = 0;
	int rsum = 0, ndgame = 0, ndsum = 0, gndsum = 0, gndsumc = 0, bdgame = 0, bdsum = 0, gbdsum = 0, gbdsumc = 0;
	int bitx = 0, bitc = 0, shad = 0, shadc = 0, hghs = 0, hghsc = 0;
	int rgbd = 0, rgbdc = 0;
	static int flags[20], romsize[10];
	static int numcpu[4][CPU_COUNT], numsnd[4][SOUND_COUNT], sumcpu[MAX_CPU+1], sumsound[MAX_SOUND+1];
	static int resx[400], resy[400], resnum[400];
	static int palett[300], palettnum[300], control[35];
	static int fpsnum[50];
	float fps[50];

	*buffer = 0;

	strcat(buffer, APPNAME " ");
	strcat(buffer, build_version);

	for (i = 0; drivers[i]; i++)
	{ 
		int controltmp[6];
		machine_config drv;
		expand_machine_driver(drivers[i]->drv, &drv);
 
		all++;

		clone_of = driver_get_clone(drivers[i]);
		if (clone_of && !(clone_of->flags & NOT_A_DRIVER))
		{
			clone = 1;
			cl++;
		}
		else
			clone = 0;


		/* Calc all graphic resolution and aspect ratio numbers */
		if (drivers[i]->flags & ORIENTATION_SWAP_XY)
		{
			x = drv.screen[0].defstate.visarea.max_y - drv.screen[0].defstate.visarea.min_y + 1;
			y = drv.screen[0].defstate.visarea.max_x - drv.screen[0].defstate.visarea.min_x + 1;
		}
		else
		{
			x = drv.screen[0].defstate.visarea.max_x - drv.screen[0].defstate.visarea.min_x + 1;
			y = drv.screen[0].defstate.visarea.max_y - drv.screen[0].defstate.visarea.min_y + 1;
		}

		if (drv.video_attributes & VIDEO_TYPE_VECTOR)
		{
	 		vec++;
			if (clone) vecc++;
		}
		else
		{
			/* Store all resolutions, without the Vector games */
			for (n = 0; n < 200; n++)
			{
				if (resx[n] == x && resy[n] == y)
				{
					resnum [n]++;
					break;
				}

				if (resx[n] == 0)
				{
					resx[n] = x;
					resy[n] = y;
					resnum [n]++;
					break;
				}

			}

		}

		/* Calc all palettesize numbers */
		x = drv.total_colors;
		for (n = 0; n < 150; n++)
		{
			if (palett[n] == x)
			{
				palettnum [n]++;
				break;
			}

			if (palett[n] == 0)
			{
				palett[n] = x;
				palettnum [n]++;
				break;
			}
		}


		if (!mame_stricmp (drivers[i]->source_file+12, "neogeo.c"))
		{
			neo++;
			if (clone) neoc++;
		}

		if (!mame_stricmp (drivers[i]->source_file+12, "playch10.c"))
		{
			pch++;
			if (clone) pchc++;
		}

		if (!mame_stricmp (drivers[i]->source_file+12, "decocass.c"))
		{
			deco++;
			if (clone) decoc++;
		}

		if (!mame_stricmp (drivers[i]->source_file+12, "cvs.c"))
		{
			cvs++;
			if (clone) cvsc++;
		}

		if (drivers[i]->flags & ORIENTATION_SWAP_XY)
		{
	 		vertical++;
			if (clone) verticalc++;
		}
		else
		{
			horizontal++;
			if (clone) horizontalc++;
		}

		x = 0;
		for (y = 0; y < MAX_SPEAKER; y++)
			if (drv.speaker[y].tag != NULL)
				x++;

		if (x > 1)
		{
	 		stereo++;
			if (clone) stereoc++;
		}


		/* Calc GAME INPUT and numbers */
		n = 0, x = 0, y = 0;
		memset(controltmp, 0, sizeof controltmp);
		
		if (drivers[i]->construct_ipt)
		{
			begin_resource_tracking();
		
			inp = input_port_allocate(drivers[i]->construct_ipt, NULL);
			while (inp->type != IPT_END)
			{
				if (n < inp->player + 1)
					n = inp->player + 1;

				switch (inp->type)
				{
				case IPT_JOYSTICK_LEFT:
				case IPT_JOYSTICK_RIGHT:
					if (!y)
						y = 1;
					break;
				case IPT_JOYSTICK_UP:
				case IPT_JOYSTICK_DOWN:
					if (inp->four_way)
						y = 2;
					else
						y = 3;
					break;
				case IPT_JOYSTICKRIGHT_LEFT:
				case IPT_JOYSTICKRIGHT_RIGHT:
				case IPT_JOYSTICKLEFT_LEFT:
				case IPT_JOYSTICKLEFT_RIGHT:
					if (!y)
						y = 4;
					break;
				case IPT_JOYSTICKRIGHT_UP:
				case IPT_JOYSTICKRIGHT_DOWN:
				case IPT_JOYSTICKLEFT_UP:
				case IPT_JOYSTICKLEFT_DOWN:
					if (inp->four_way)
						y = 5;
					else
						y = 6;
					break;
				case IPT_BUTTON1:
					if (x<1) x = 1;
					break;
				case IPT_BUTTON2:
					if (x<2) x = 2;
					break;
				case IPT_BUTTON3:
					if (x<3) x = 3;
					break;
				case IPT_BUTTON4:
					if (x<4) x = 4;
					break;
				case IPT_BUTTON5:
					if (x<5) x = 5;
					break;
				case IPT_BUTTON6:
					if (x<6) x = 6;
					break;
				case IPT_BUTTON7:
					if (x<7) x = 7;
					break;
				case IPT_BUTTON8:
					if (x<8) x = 8;
					break;
				case IPT_BUTTON9:
					if (x<9) x = 9;
					break;
				case IPT_BUTTON10:
					if (x<10) x = 10;
					break;
				case IPT_PADDLE:
					controltmp[0] = 1;
					break;
				case IPT_DIAL:
					controltmp[1] = 1;
					break;
				case IPT_TRACKBALL_X:
				case IPT_TRACKBALL_Y:
					controltmp[2] = 1;
					break;
				case IPT_AD_STICK_X:
				case IPT_AD_STICK_Y:
					controltmp[3] = 1;
					break;
				case IPT_LIGHTGUN_X:
				case IPT_LIGHTGUN_Y:
					controltmp[4] = 1;
					break;
				case IPT_PEDAL:
					controltmp[5] = 1;
					break;
				}
				++inp;
			}
			end_resource_tracking();
		}	
		if (n) control[n]++;
		if (x) control[x+10]++;
		if (y) control[y+20]++;

		if (!y)
		{
			noinput++;
			for (y = 0; y < sizeof (controltmp) / sizeof (*controltmp); y++)
				if (controltmp[y])
				{
					noinput--;
					break;
				}
		}

		for (y = 0; y < sizeof (controltmp) / sizeof (*controltmp); y++)
			if (controltmp[y])
				control[y + 27]++;


		/* Calc all Frames_Per_Second numbers */
		fps[0] = drv.screen[0].defstate.refresh;
		for (n = 1; n < 50; n++)
		{
			if (fps[n] == fps[0])
			{
				fpsnum[n]++;
				break;
			}

			if (fpsnum[n] == 0)
			{
				fps[n] = fps[0];
				fpsnum[n]++;
				fpsnum[0]++;
				break;
			}
		}


		/* Calc number of various info 'flags' in original and clone games */
		if (mame_stricmp (drivers[i]->source_file+12, "neogeo.c"))
		{
			if (drivers[i]->flags & GAME_NOT_WORKING)
			{
	 			flags[1]++;
				if (clone) flags[11]++;
			}

			if (drivers[i]->flags & GAME_UNEMULATED_PROTECTION)
			{
	 			flags[2]++;
				if (clone) flags[12]++;
			}
			if (drivers[i]->flags & GAME_IMPERFECT_GRAPHICS)
			{
	 			flags[3]++;
				if (clone) flags[13]++;
			}
			if (drivers[i]->flags & GAME_WRONG_COLORS)
			{
	 			flags[4]++;
				if (clone) flags[14]++;
			}
			if (drivers[i]->flags & GAME_IMPERFECT_COLORS)
			{
	 			flags[5]++;
				if (clone) flags[15]++;
			}
			if (drivers[i]->flags & GAME_NO_SOUND)
			{
	 			flags[6]++;
				if (clone) flags[16]++;
			}
			if (drivers[i]->flags & GAME_IMPERFECT_SOUND)
			{
	 			flags[7]++;
				if (clone) flags[17]++;
			}
			if (drivers[i]->flags & GAME_NO_COCKTAIL)
			{
	 			flags[8]++;
				if (clone) flags[18]++;
			}

			if (drv.video_attributes & VIDEO_NEEDS_6BITS_PER_GUN)
			{
	 			bitx++;
				if (clone) bitc++;
			}

			if (drv.video_attributes & VIDEO_RGB_DIRECT)
			{
	 			rgbd++;
				if (clone) rgbdc++;
			}

			if (drv.video_attributes & VIDEO_HAS_SHADOWS)
			{
	 			shad++;
				if (clone) shadc++;
			}

			if (drv.video_attributes & VIDEO_HAS_HIGHLIGHTS)
			{
	 			hghs++;
				if (clone) hghsc++;
			}


			if (!clone)
			{
				/* Calc all CPU's only in ORIGINAL games */
				y = 0;
				n = 0;
				while (n < MAX_CPU && drv.cpu[n].cpu_type)
				{
					int type = drv.cpu[n].cpu_type;
					int count = 0;
					char cpu_name[256];

					n++;

					while (n < MAX_CPU
						&& drv.cpu[n].cpu_type == type)
					{
						count++;
						n++;
					}

					strcpy(cpu_name, cputype_shortname(type));
					if (cpu_name[0] == '\0')
						continue;

					if (count < 4)
					{
						numcpu[count][type]++;
						y++;
					}
				}

				sumcpu[y]++;


				/* Calc all Sound hardware only in ORIGINAL games */
				y = 0;
				n = 0;
				while (n < MAX_SOUND && drv.sound[n].sound_type)
				{
					int type = drv.sound[n].sound_type;
					int count = 0;
					char sound_name[256];

					n++;

					while (n < MAX_CPU
						&& drv.sound[n].sound_type == type)
					{
						count++;
						n++;
					}

					strcpy(sound_name, sndtype_shortname(type));
					if (sound_name[0] == '\0')
						continue;

					if (type == SOUND_FILTER_VOLUME
					 || type == SOUND_FILTER_RC
					 || type == SOUND_FILTER_LOWPASS)
						continue;

					if (count < 4)
					{
						numsnd[count][type]++;
						y++;
					}
				}

				sumsound[y]++;
			}
		}


		/* Calc number of ROM files and file size */
		for (region = rom_first_region(drivers[i]); region; region = rom_next_region(region))
			for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
			{
				int length, in_parent, is_disk;
				is_disk = ROMREGION_ISDISKDATA(region);



				length = 0;
				in_parent = 0;

				for (chunk = rom_first_chunk(rom); chunk; chunk = rom_next_chunk(chunk))
					length += ROM_GETLENGTH(chunk)/32;

				if (clone_of)
				{
					fprom=NULL;
					for (pregion = rom_first_region(clone_of); pregion; pregion = rom_next_region(pregion))
						for (prom = rom_first_file(pregion); prom; prom = rom_next_file(prom))
							if (hash_data_is_equal(ROM_GETHASHDATA(rom), ROM_GETHASHDATA(prom), 0))
							{
								if (!fprom || !strcmp(ROM_GETNAME(prom), name))
									fprom=prom;
								in_parent = 1;
							}

				}

				if (!is_disk)
				{
					sum += length;
					rsum += length;
					files++;

					if(in_parent)
					{
						xsum += length;
						mfiles++;
					}
				}
				else
					hddisk++;

				if (hash_data_has_info(ROM_GETHASHDATA(rom), HASH_INFO_NO_DUMP))
				{
					ndsum++;
					ndgame = 1;
				}

				if (hash_data_has_info(ROM_GETHASHDATA(rom), HASH_INFO_BAD_DUMP))
				{
					bdsum++;
					bdgame = 1;
				}

			}

		if (ndgame)
		{
			gndsum++;
			if (clone) gndsumc++;
			ndgame = 0;
		}

		if (bdgame)
		{
			gbdsum++;
			if (clone) gbdsumc++;
			bdgame = 0;
		}


		rsum = rsum/32;
		if(rsum < 5)
			romsize[0]++;
		if(rsum >= 5 && rsum < 10)
			romsize[1]++;
		if(rsum >= 10 && rsum < 50)
			romsize[2]++;
		if(rsum >= 50 && rsum < 100)
			romsize[3]++;
		if(rsum >= 100 && rsum < 500)
			romsize[4]++;
		if(rsum >= 500 && rsum < 1000)
			romsize[5]++;
		if(rsum >= 1000 && rsum < 5000)
			romsize[6]++;
		if(rsum >= 5000 && rsum < 10000)
			romsize[7]++;
		if(rsum >= 10000 && rsum < 50000)
			romsize[8]++;
		if(rsum >= 50000)
			romsize[9]++;
		rsum = 0;

	}

	sum = sum/32;
	xsum = xsum/32;
	noyear = all;		/* See Print Year and Games */
	// already calculated.
	//noinput = all;		/* See Input */
	nobutt = all;		/* See Input Buttons */


	sprintf(name, _("\n\n %4d GAMES (ALL)\n %4d ORIGINALS  + %4d CLNS\n %4d NEOGEO     + %4d\n"), all, all-cl-neo+neoc, cl-neoc, neo-neoc, neoc);
	strcat(buffer, name);
	sprintf(name, _(" %4d PLAYCHOICE + %4d\n %4d DECO CASS  + %4d\n %4d CVS        + %4d\n"), pch-pchc, pchc, deco-decoc, decoc, cvs-cvsc, cvsc);
	strcat(buffer, name);
	sprintf(name, _(" %4d RASTER     + %4d\n %4d VECTOR     + %4d\n"), all-vec-cl+vecc, cl-vecc, vec-vecc, vecc);
	strcat(buffer, name);
	sprintf(name, _(" %4d HORIZONTAL + %4d\n %4d VERTICAL   + %4d\n"), horizontal-horizontalc, horizontalc, vertical-verticalc, verticalc);
	strcat(buffer, name);
	sprintf(name, _(" %4d STEREO     + %4d\n"), stereo-stereoc, stereoc);
	strcat(buffer, name);
	sprintf(name, _(" %4d HARDDISK\n"), hddisk);
	strcat(buffer, name);

	/* Print number of various info 'flags' */
	strcat(buffer,_("\n\nGAME INFO FLAGS   : ORIG CLNS\n\n"));
	sprintf(name, _("NON-WORKING        : %3d  %3d\n"), flags[1]-flags[11], flags[11]);
	strcat(buffer, name);
	sprintf(name, _("UNEMULATED PROTEC. : %3d  %3d\n"), flags[2]-flags[12], flags[12]);
	strcat(buffer, name);
	sprintf(name, _("IMPERFECT GRAPHICS : %3d  %3d\n"), flags[3]-flags[13], flags[13]);
	strcat(buffer, name);
	sprintf(name, _("WRONG COLORS       : %3d  %3d\n"), flags[4]-flags[14], flags[14]);
	strcat(buffer, name);
	sprintf(name, _("IMPERFECT COLORS   : %3d  %3d\n"), flags[5]-flags[15], flags[15]);
	strcat(buffer, name);
	sprintf(name, _("NO SOUND           : %3d  %3d\n"), flags[6]-flags[16], flags[16]);
	strcat(buffer, name);
	sprintf(name, _("IMPERFECT SOUND    : %3d  %3d\n"), flags[7]-flags[17], flags[17]);
	strcat(buffer, name);
	sprintf(name, _("NO COCKTAIL        : %3d  %3d\n"), flags[8]-flags[18], flags[18]);
	strcat(buffer, name);
	sprintf(name, _("NO GOOD DUMP KNOWN : %3d  %3d\n(%3d ROMS IN %d GAMES)\n"), gndsum-gndsumc, gndsumc, ndsum, gndsum);
	strcat(buffer, name);
	sprintf(name, _("ROM NEEDS REDUMP   : %3d  %3d\n(%3d ROMS IN %d GAMES)\n"), gbdsum-gbdsumc, gbdsumc, bdsum, gbdsum);
	strcat(buffer, name);




	/* Print Year and Games - Note: Some games have no year*/
	strcat(buffer,_("\n\nYEAR: ORIG  CLNS NEOGEO  ALL\n\n"));
	for (x = 1972; x < 2004; x++)
	{

		all = 0; cl = 0; neo = 0; neoc = 0;

		sprintf(year,"%d",x);
		for (i = 0; drivers[i]; i++)
		{
			if (!mame_stricmp (year, drivers[i]->year))
			{ 
				all++;

				if (clone_of && !(clone_of->flags & NOT_A_DRIVER))
					cl++;
				if (!mame_stricmp (drivers[i]->source_file+12, "neogeo.c"))
				{
					neo++;
					if (clone_of && !(clone_of->flags & NOT_A_DRIVER))
						neoc++;
				}
			}
		}

		sprintf(name, "%d: %3d   %3d   %3d    %3d\n", x, all-cl-neo+neoc, cl-neoc, neo, all);
		strcat(buffer, name);

		noyear = noyear - all;	/* Number of games with no year informations */

	}
	sprintf(name, "19??:                    %3d\n\n", noyear);
	strcat(buffer, name);


	/* Print number of ROM files and whole file size */
	if(sum > 524288)
		sprintf(name, _("\nROMFILES: %d\n%d + %d MERGED ROMS\n\nFILESIZE: %d MB (UNCOMPRESSED)\n%d MB + %d MB MERGED\n"), files, files-mfiles, mfiles, sum/1024, sum/1024-xsum/1024, xsum/1024);
	else
		sprintf(name, _("\n\nROMFILES: %d\n%d + %d MERGED ROMS\n\nFILESIZE: %d KB (UNCOMPRESSED)\n%d KB + %d KB MERGED\n"), files, files-mfiles, mfiles, sum, sum-xsum, xsum);
	strcat(buffer, name);



	/* Print the typical sizes of all supported romsets in kbytes */
	strcat(buffer,_("\n\nTYPICAL ROMSET SIZE (ALL)\n"));

	sprintf(name, "\n    0 -     5 KB:  %4d\n    5 -    10 KB:  %4d\n   10 -    50 KB:  %4d\n   50 -   100 KB:  %4d\n", romsize[0], romsize[1], romsize[2], romsize[3]);
	strcat(buffer, name);
	sprintf(name, "  100 -   500 KB:  %4d\n  500 -  1000 KB:  %4d\n 1000 -  5000 KB:  %4d\n 5000 - 10000 KB:  %4d\n", romsize[4], romsize[5], romsize[6], romsize[7]);
	strcat(buffer, name);
	sprintf(name, "10000 - 50000 KB:  %4d\n      > 50000 KB:  %4d\n", romsize[8], romsize[9]);
	strcat(buffer, name);



	/* Print year and the number of added games */
	strcat(buffer,_("\n\nYEAR    NEW GAMES\n\n"));
	for (i = 0; stat_newgames[i]; i++)
	{
		strcat(buffer,stat_newgames[i]);
		strcat(buffer,"\n");
	}



	/* Print all games and their maximum CPU's */
	strcat(buffer,_("\n\nCPU HARDWARE (ORIGINAL GAMES)\n\n"));
	for (n = 0; n < MAX_CPU+1; n++)
	{
		if (sumcpu[n])
		{
			sprintf(name, _(" GAMES WITH  %d CPUs: %4d\n"), n, sumcpu[n]);
			strcat(buffer, name);
		}

	}


	/* Print all used CPU's and numbers of original games they used */
	strcat(buffer,"\n         CPU:  (1)  (2)  (3)  (4)\n\n");
	all = 0;
	for (i = 1; i < CPU_COUNT; i++)
	{
		if (numcpu[0][i] || numcpu[1][i] || numcpu[2][i] || numcpu[3][i])
		{
			sprintf(name, "%12s: %4d %4d %4d %4d\n", cputype_shortname(i), numcpu[0][i], numcpu[1][i], numcpu[2][i], numcpu[3][i]);
			strcat(buffer, name);
			all = all + numcpu[0][i] + numcpu[1][i] + numcpu[2][i] + numcpu[3][i];
		}
	}

	/* Print the number of all CPU the original games */
	sprintf(name, _("\n   TOTAL: %4d\n"), all);
	strcat(buffer, name);


	/* Print all games and their maximum number of sound subsystems */
	strcat(buffer,_("\n\nSOUND SYSTEM (ORIGINAL GAMES)\n\n"));
	for (n = 0; n < MAX_SOUND+1; n++)
	{
		if (sumsound[n])
		{
			sprintf(name, _(" GAMES WITH  %d SNDINTRF: %4d\n"), n, sumsound[n]);
			strcat(buffer, name);
		}

	}

	/* Print all Sound hardware and numbers of games they used */
	strcat(buffer,_("\n    SNDINTRF:  (1)  (2)  (3)  (4)\n\n"));
	all = 0;
	for (i = 1; i < SOUND_COUNT; i++)
	{
		if (numsnd[0][i] || numsnd[1][i] || numsnd[2][i] || numsnd[3][i])
		{
			sprintf(name, "%12s: %4d %4d %4d %4d\n", sndtype_shortname(i), numsnd[0][i], numsnd[1][i], numsnd[2][i], numsnd[3][i]);
			strcat(buffer, name);
			all = all + numsnd[0][i] + numsnd[1][i] + numsnd[2][i] + numsnd[3][i];
		}
	}

	sprintf(name, _("\n    TOTAL: %4d\n"), all);
	strcat(buffer, name);


	/* Print all Input Controls and numbers of all games */
	strcat(buffer, _("\n\nCABINET INPUT CONTROL: (ALL)\n\n"));
	for (n = 1; n < 9; n++)
	{
		if (control[n])
		{

			sprintf(name, _("     PLAYERS %d:  %4d\n"), n, control[n]);
			strcat(buffer, name);
		}
	}

	strcat(buffer, "\n");
	if (control[21]) { sprintf(name, _("       JOY2WAY:  %4d\n"), control[21]); strcat(buffer, name); }
	if (control[22]) { sprintf(name, _("       JOY4WAY:  %4d\n"), control[22]); strcat(buffer, name); }
	if (control[23]) { sprintf(name, _("       JOY8WAY:  %4d\n"), control[23]); strcat(buffer, name); }
	if (control[24]) { sprintf(name, _(" DOUBLEJOY2WAY:  %4d\n"), control[24]); strcat(buffer, name); }
	if (control[25]) { sprintf(name, _(" DOUBLEJOY4WAY:  %4d\n"), control[25]); strcat(buffer, name); }
	if (control[26]) { sprintf(name, _(" DOUBLEJOY8WAY:  %4d\n"), control[26]); strcat(buffer, name); }
	if (control[27]) { sprintf(name, _("        PADDLE:  %4d\n"), control[27]); strcat(buffer, name); }
	if (control[28]) { sprintf(name, _("          DIAL:  %4d\n"), control[28]); strcat(buffer, name); }
	if (control[29]) { sprintf(name, _("     TRACKBALL:  %4d\n"), control[29]); strcat(buffer, name); }
	if (control[30]) { sprintf(name, _("      AD STICK:  %4d\n"), control[30]); strcat(buffer, name); }
	if (control[31]) { sprintf(name, _("      LIGHTGUN:  %4d\n"), control[31]); strcat(buffer, name); }
	if (control[32]) { sprintf(name, _("         PEDAL:  %4d\n"), control[32]); strcat(buffer, name); }

	sprintf(name, _("         OTHER:  %4d\n"), noinput);
	strcat(buffer, name);

	strcat(buffer, "\n");
	for (n = 11; n < 21; n++)
	{
		if (control[n])
		{

			sprintf(name, _("    BUTTONS%3d:  %4d\n"), n-10, control[n]);
			strcat(buffer, name);
			nobutt = nobutt - control[n];			
		}
	}

	sprintf(name, _("    NO BUTTONS:  %4d\n"), nobutt);
	strcat(buffer, name);


	/* Print the video_attributes */
	strcat(buffer,_("\n\nVIDEO NEEDS... : ORIG   CLNS\n\n"));
	sprintf(name, _("24-BIT DISPLAY : %3d  + %3d\n"), bitx-bitc, bitc);
	strcat(buffer, name);
	sprintf(name, _("HI/TRUE BITMAP : %3d  + %3d\n"), rgbd-rgbdc, rgbdc);
	strcat(buffer, name);
	sprintf(name, _("SHADOWS        : %3d  + %3d\n"), shad-shadc, shadc);
	strcat(buffer, name);
	sprintf(name, _("HIGHLIGHTS     : %3d  + %3d\n"), hghs-hghsc, hghsc);
	strcat(buffer, name);


	/* FRAMES_PER_SECOND: Sort and print all fps */
	sprintf(name,_("\n\nFRAMES PER SECOND (%d): (ALL)\n\n"), fpsnum[0]);
	strcat(buffer, name);
	for (y = 1; y < 50; y++)
	{
		fps[0] = 199;
		for (n = 1; n < 50; n++)
		{
			if (fpsnum[n] && fps[0] > fps[n])
				fps[0] = fps[n];
		}

		for (n = 1; n < 50; n++)	/* Print fps and number*/
		{
			if (fps[0] == fps[n])
			{
				sprintf(name, "  FPS %f:  %4d\n", fps[n], fpsnum[n]);
				strcat(buffer, name);
				fpsnum[n] = 0;
			}
		}

	}

	if (fpsnum[0] > 48)
		strcat(buffer, "\nWARNING: FPS number too high!\n");


	/* RESOLUTION: Sort all games resolutions by x and y */
	cl = 0;
	for (all = 0; all < 200; all++)
	{
		x = 999;
		for (n = 0; n < 200; n++)
		{
			if (resx[n] && x > resx[n])
				x = resx[n];
		}

		y = 999;
		for (n = 0; n < 200; n++)
		{
			if (x == resx[n] && y > resy[n])
				y = resy[n];
		}

		for (n = 0; n < 200; n++)
		{
			if (x == resx[n] && y == resy[n])
			{
				/* Store all sorted resolutions in the higher array */
				resx[200+cl] = resx[n];
				resy[200+cl] = resy[n];
				resnum[200+cl] = resnum[n];
				cl++;
				resx[n] = 0, resy[n] = 0, resnum[n] = 0;
			}
		}
	}


	/* PALETTESIZE: Sort the palettesizes */
	x = 0;
	for (y = 0; y < 150; y++)
	{
		i = 99999;
		for (n = 0; n < 150; n++)
		{
			if (palett[n] && i > palett[n])
				i = palett[n];
		}

		for (n = 0; n < 150; n++)	/* Store all sorted resolutions in the higher array */
		{
			if (i == palett[n])
			{
				palett[150+x] = palett[n];
				palettnum[150+x] = palettnum[n];
				x++;
				palett[n] = 0, palettnum[n] = 0;
			}
		}
	}

	/* RESOLUTION & PALETTESIZE: Print all resolutions and palettesizes */
	sprintf(name,_("\n\nRESOLUTION & PALETTESIZE: (ALL)\n    (%d)          (%d)\n\n"), cl, x);
	strcat(buffer, name);
	for (n = 0; n < 200; n++)
	{

		if (resx[n+200])
		{
			sprintf(name, "  %dx%d: %3d    ", resx[n+200], resy[n+200], resnum[n+200]);
			strcat(buffer, name);
		}

		if (n < 150 && palett[n+150])
		{
			sprintf(name, "%5d: %3d\n", palett[n+150], palettnum[n+150]);
			strcat(buffer, name);
		}
		else
			if (resx[n+200])	strcat(buffer, "\n");

	}

	if (cl > 198)
		strcat(buffer, "\nWARNING: Resolution number too high!\n");

	if (x > 148)
		strcat(buffer, "\nWARNING: Palettesize number too high!\n");


	/* MAME HISTORY - Print all MAME versions + Drivers + Supported Games (generated with MAMEDiff) */
	strcat(buffer,_("\n\nVERSION - DRIVERS - SUPPORT:\n\n"));
	for (i = 0; stat_history[i]; i++)
	{
		strcat(buffer,stat_history[i]);
		strcat(buffer,"\n");
	}


	/* Calc all MAME versions and print all version */
	for (i = 0; stat_versions[i]; i++){}
	sprintf(name, _("\n\nMAME VERSIONS (%3d)\n\n"), i);
	strcat(buffer, name);

	for (i = 0; stat_versions[i]; i++)
	{
		strcat(buffer,stat_versions[i]);
		strcat(buffer,"\n");
	}


	/* CLEAR ALL DATA ARRAYS */
	memset(numcpu, 0, sizeof numcpu);
	memset(numsnd, 0, sizeof numsnd);
	memset(resx, 0, sizeof resx);
	memset(resy, 0, sizeof resy);
	memset(resnum, 0, sizeof resnum);
	memset(palett, 0, sizeof palett);
	memset(palettnum, 0, sizeof palettnum);

	memset(flags, 0, sizeof flags);
	memset(romsize, 0, sizeof romsize);
	memset(control, 0, sizeof control);
	memset(fps, 0, sizeof fps);
	memset(fpsnum, 0, sizeof fpsnum);
	memset(sumcpu, 0, sizeof sumcpu);
	memset(sumsound, 0, sizeof sumsound);

	return 0;

}


#ifdef CMD_LIST
/**************************************************************************
 *	find_command
 **************************************************************************/
static int find_command (const game_driver *drv)
{
	int where;
	int i;

	flush_index_if_needed();

	if (!command_filename)
		command_filename = "command.dat";

	if (!lang_directory)
		lang_directory = "lang";

	if (menu_filename)
		free(menu_filename);

	for (where = 0; where <= FILE_ROOT; where += FILE_ROOT)
	{
		char filename[80];
		char *base;

		filename[0] = '\0';

		if (where != FILE_ROOT)
		{
			sprintf(filename, "%s\\%s\\",
		        	lang_directory,
				ui_lang_info[options.langcode].name);
		}

		base = filename + strlen(filename);

		for (i = where; i <= where + FILE_MERGED; i += FILE_MERGED)
		{
			int status = 0;

			if (i & FILE_MERGED)
			{
				if (where & FILE_ROOT)
					strcpy(base, command_filename);
				else
					strcpy(base, "command.dat");

				/* try to open command datafile */
				if (!ParseOpen (filename))
					continue;

				/* create index if necessary */
				if (cmnd_idx[i])
					status = 1;
				else
				{
					status = (index_datafile (&cmnd_idx[i]) != 0);
					free_menuidx(&menu_idx);
				}

				/* create menu_index */
				status = (index_menuidx (drv, cmnd_idx[i], &menu_idx) != 0);

				if (!status)
					free_menuidx(&menu_idx);

				ParseClose ();
			}
			else
			{
				const game_driver *gdrv;

				for (gdrv = drv; !status && gdrv && gdrv->name[0]; gdrv = driver_get_clone(gdrv))
				{
					strcpy(base, "command\\");
					strcat(base, gdrv->name);
					strcat(base, ".dat");

					/* try to open command datafile */
					if (!ParseOpen (filename))
						continue;

					if (cmnd_idx[i])
					{
						free(cmnd_idx[i]);
						cmnd_idx[i] = 0;
					}

					status = (index_datafile (&cmnd_idx[i]) != 0);
					free_menuidx(&menu_idx);

					/* create menu_index */
					status = (index_menuidx (drv, cmnd_idx[i], &menu_idx) != 0);

					if (!status)
						free_menuidx(&menu_idx);

					ParseClose ();
				}
			}

			if (status)
			{
				menu_filename = mame_strdup(filename);

				return 0;
			}
		}
	}

	return 1;
}


/**************************************************************************
 *	load_driver_command_ex
 **************************************************************************/
int load_driver_command_ex (const game_driver *drv, char *buffer, int bufsize, const int menu_sel)
{
	*buffer = 0;

	//if (find_command (drv))
	//	return 1;

	if (!menu_filename)
		return 1;

	/* try to open command datafile */
	if (ParseOpen (menu_filename))
	{
		int err;

		err = load_datafile_text_ex (buffer, bufsize,
		                             DATAFILE_TAG_COMMAND, menu_idx, menu_sel);

		ParseClose ();

		if (!err)
			return 0;
	}

	return 1;
}


/**************************************************************************
 *	command_sub_menu
 **************************************************************************/
UINT8 command_sub_menu(const game_driver *drv, const char *menu_item[])
{
	if (find_command (drv))
		return 0;

	if (menu_idx)
	{
		struct tMenuIndex *m_idx = menu_idx;
		int total = 0;

		while(m_idx->menuitem != NULL)
		{
			menu_item[total++] = m_idx->menuitem;
			m_idx++;
		}

		return total;
	}

	return 0;
}
#endif /* CMD_LIST */
