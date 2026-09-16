function [alpha, beta] = p2_4(x, y)
X = [x(:) ones(length(x),1)];
params = X\y(:);
alpha = params(1);
beta = params(2);
end

[alpha, beta] = p2_4(x, y);
disp([alpha, beta])