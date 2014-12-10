# PURPOSE

+ locate\_block: 把中選會競選公報底圖裡的「格子」找出來
+ guess\_field.pl: 根據「格子」的坐標分佈去猜是不是候選人資料以及內容應該是什麼

# DEPENDENCY

+ libpng
+ Perl

# INSTALL

```
$ make
```

# SYNOPSIS

```
$ ./locate_block < input.png > output.txt
$ perl guess_field.pl < output.txt > output.json
```
or
```
$ ./locate_block < input.png | perl guess_field.pl > output.json
```

# OUTPUT FORMAT

output.txt
```
x1 y1 width1 height1
x2 y2 width2 height2
...
```

output.json (如果猜得正確的話, 會依該頁公報中候選人的順序排列)
```json
[ { "號次":
    [x, y, width, height]
  , "相片":
    [x, y, width, height]
  , "姓名":
    [x, y, width, height]
  , "出生年月日":
    [x, y, width, height]
  , "性別":
    [x, y, width, height]
  , "出生地":
    [x, y, width, height]
  , "推薦之政黨":
    [x, y, width, height]
  , "學歷":
    [x, y, width, height]
  , "經歷":
    [x, y, width, height]
  , "政見":
    [x, y, width, height]
  }
, { ...
  }
...
]
```

# SAMPLE
+ [sample input](https://github.com/CindyLinz/BulletinCEC-LocateBlockFromPNG/blob/master/input.png)
+ [sample output (txt)](https://github.com/CindyLinz/BulletinCEC-LocateBlockFromPNG/blob/master/output.txt)
+ [sample output (json)](https://github.com/CindyLinz/BulletinCEC-LocateBlockFromPNG/blob/master/output.json)

# LICENSE

Copyright 2014, Cindy Wang (CindyLinz) Licensed under the MIT license.

