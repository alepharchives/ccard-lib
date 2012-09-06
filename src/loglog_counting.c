#include "loglog_counting.h"

/**
 * Gamma function computed using SciLab
 * ((gamma(-(m.^(-1))).* ( (1-2.^(m.^(-1)))./log(2) )).^(-m)).*m
 * */
static const double[] alpha = {
	0,
	0.44567926005415,
	1.2480639342271,
	2.8391255240079,
	6.0165231584811,
	12.369319965552,
	25.073991603109,
	50.482891762521,
	101.30047482549,
	202.93553337953,
	406.20559693552,
	812.74569741657,
	1625.8258887309,
	3251.9862249084,
	6504.3071471860,
	13008.949929672,
	26018.222470181,
	52036.684135280,
	104073.41696276,
	208139.24771523,
	416265.57100022,
	832478.53851627,
	1669443.2499579,
	3356902.8702907,
	6863377.8429508,
	11978069.823687,
	31333767.455026,
	52114301.457757,
	72080129.928986,
	68945006.880409,
	31538957.552704,
	3299942.4347441
};

struct ll_cnt_ctx_s {
	int err;
	uint8_t k;
	uint32_t m;
	double Ca;
	uint32_t Rsum;
	uint8_t M[1];
};

static uint8_t num_of_trail_zeros(uint32_t i)
{
	uint32_t y;
	uint8_t n = 31;

	if (i == 0)
		return 32;

	y = i << 16;	if (y != 0) { n -= 16; i = y; }
	y = i << 8;		if (y != 0) { n -= 8; i = y; }
	y = i << 4;		if (y != 0) { n -= 4; i = y; }
	y = i << 2;		if (y != 0) { n -= 2; i = y; }
	return n - (uint8_t)((i << 1) >> 31);
}

ll_cnt_ctx_t* ll_cnt_init(const void *buf, uint32_t len_or_k)
{
	ll_cnt_ctx_t *ctx;

	if (len_or_k == 0) {
		// invalid buffer length or k
		return NULL;
	}

	if (buf) {
		// initial bitmap was given
		uint32_t i;
		uint8_t k = num_of_trail_zeros(len_or_k);

		if (len_or_k != (1 << k)) {
			// invalid buffer size, its length must be a power of 2
			return NULL;
		}

		ctx = malloc(sizeof(ll_cnt_ctx_t) + len_or_k - 1);
		ctx->err = CCARD_OK;
		memcpy(ctx->M, buf, len_or_k);
		ctx->m = len_or_k;
		ctx->k = k;
		ctx->Ca = alpha[k];
		ctx->Rsum = 0;
		for(i = 0; i < len_or_k; ++i) {
			ctx->Rsum += buf[i];
		}
	} else {
		// k was given
		if (len_or_k >= sizeof(alpha)/sizeof(alpha[0])) {
			// exceeded maximum k
			return NULL;
		}

		ctx = malloc(sizeof(ll_cnt_ctx_t) + (1 << len_or_k) - 1);
		ctx->err = CCARD_OK;
		ctx->m = 1 << len_or_k;
		ctx->k = (uint8_t)len_or_k;
		ctx->Ca = alpha[len_or_k];
		memset(ctx->M, 0, ctx->m);
	}

	return ctx;
}

int ll_cnt_fini(ll_cnt_ctx_t *ctx)
{
	if (ctx) {
		free(ctx);
		return 0;
	}

	return -1;
}

int ll_cnt_errnum(ll_cnt_ctx_t *ctx)
{
	if (ctx) {
		return ctx->err;
	}

	return CCARD_ERR_INVALID_CTX;
}

const char* ll_cnt_errstr(int err)
{
	static const char *msg[] = {
		"No error",
		"Invalid algorithm context",
		NULL
	};

	if (-err >= 0 && -err < sizeof(msg)/sizeof(msg[0])-1) {
		return msg[-err];
	}

	return "Invalid error number";
}
