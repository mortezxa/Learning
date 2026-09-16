data = cell(2,32);

row1 = 'abcdefghijklmnopqrstuvwxyz .,!:"';
for i = 1:length(row1)
    data{1, i} = row1(i);
end

for i = 0:31
    data{2, i+1} = dec2bin(i, 5);
end

save('Mapset.mat', 'data');

