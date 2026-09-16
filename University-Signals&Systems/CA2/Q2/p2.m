clc; clear; close all;
[fname, fpath] = uigetfile({'*.jpg;*.bmp;*.png;*.tif'}, 'Select a plate image');
if isequal(fname,0)
    return;
end
imgPath = fullfile(fpath, fname);
carImg = imresize(imread(imgPath), [300 500]);
figure;
subplot(1,4,1); imshow(carImg);

grayImg = rgb2gray(carImg);
subplot(1,4,2); imshow(grayImg);

th = graythresh(grayImg);
bwImg = ~imbinarize(grayImg, th);
subplot(1,4,3); imshow(bwImg);

smallComp = bwareaopen(bwImg, 1500);
largeComp = bwareaopen(bwImg, 15000);
plateMask = smallComp - largeComp;
subplot(1,4,4); imshow(plateMask);

[L, N] = bwlabel(plateMask);
load PersianMapset;
nRef = size(PMap,2);
plateText = [];

for k = 1:N
    [r, c] = find(L == k);
    seg = plateMask(min(r):max(r), min(c):max(c));
    seg = imresize(seg, [150 90]);
    scores = zeros(1, nRef);
    for j = 1:nRef
        scores(j) = corr2(seg, PMap{1, j});
    end
    [mVal, mIdx] = max(scores);
    if mVal >= 0.45
        if scores(3) >= 0.79 && scores(4) >= 0.65
            ch = '3';
        else
            ch = cell2mat(PMap(2, mIdx));
        end
        if k == 7
            plateText = [plateText '|' ch];
        else
            plateText = [plateText ' ' ch];
        end
    end
end

plateText = fliplr(plateText);
fprintf('Detected Plate ID:\n%s\n', plateText);
fid = fopen('Detected_ID.txt', 'wt');
fprintf(fid, '%s\n', plateText);
fclose(fid);
winopen('Detected_ID.txt');

clc; clear; close all;
dataDir = dir('Persian Map Set');
numImgs = numel(dataDir) - 2;
PMap = cell(2, numImgs);
for n = 1:numImgs
    img = imread(fullfile(dataDir(n+2).folder, dataDir(n+2).name));
    PMap{1, n} = img;
    PMap{2, n} = dataDir(n+2).name(1);
end
save('PersianMapset', 'PMap');