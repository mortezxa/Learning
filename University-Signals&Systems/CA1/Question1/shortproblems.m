% Problem 1: Scalar variables
a = 10;                 
b = 2.5 * 10^23;        
c = 2 + 3i;             
d = exp((1i * 2 * pi)/3); 

% Problem 1.2: Vector variables
aVec = [3.14 15 9 26];
bVec = [27; 8; 28; 182];
cVec = 5 : -0.2 : -5;
dVec = 10.^(0 : 0.01 : 1);
eVec = 'Hello';

% Problem 1.3: Matrix variables
aMat = 2 * ones(9, 9);
bMat = diag([1 2 3 4 5 4 3 2 1]);
cMat = reshape(1:100, 10, 10);
dMat = NaN(3, 4);
eMat = [13 -1 5; -22 10 -87];
fMat = randi([-3, 3], 5, 3);

% Problem 1.4: Scalar sequential
x = 1 / (1 + exp(-(a-15)/6));
y = (sqrt(a) + b^(1/21))^pi;
z = log(real((c+d)*(c-d)) * sin(a * pi / 3)) / (c * conj(c));

% Problem 1.5: Matrix equations
xMat = dot(aVec, bVec) * (aMat * aMat);
yMat = bVec * aVec;
zMat = det(cMat) * (aMat * bMat)';

% Problem 1.6: Common functions and indexing
cSum = sum(cMat, 1);
eMean = mean(eMat, 2);
eMat(1, :) = [1 1 1];
cSub = cMat(2:9, 2:9);
lin = 1:20;
lin(mod(lin, 2) == 0) = -lin(mod(lin, 2) == 0);
r = rand(1, 5);
idx = find(r < 0.5);
r(idx) = 0;

% Problem 1.7:
figure;
t = linspace(0, 2*pi, 100);
plot(t, sin(t));
hold on;
plot(t, cos(t), 'r--');
xlabel('Time (t)');
ylabel('Function value');
title('Sine and Cosine Waves');
legend('Sin', 'Cos');
xlim([0 2*pi]);
ylim([-1.4 1.4]);
hold off;
