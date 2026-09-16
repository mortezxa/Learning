[c, lags] = xcorr(r, s);
[~, idx] = max(c);
td_est = lags(idx) * ts;
R_est = C * td_est / 2;

fprintf('Estimated Distance = %.2f meters\n', R_est);