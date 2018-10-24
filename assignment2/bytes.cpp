	int b = (int) floor(log2(n))
	int k = b, t = n;
	while (t > 1) { k++;  t >>= 1; }

	// Set u to the number of unused codewords = 2^(k+1) - n.
	int u = (1 << k+1) - n;

	if (x < u)  return Binary(x, k); 
        else  return Binary(x+u, k+1));
