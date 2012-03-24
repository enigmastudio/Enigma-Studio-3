

struct EventMapping
{
	eS32 control;
	eS32 param;
	eChar name[64];
};

EventMapping m_EvMapping[] = {
	{ 0x48,  TF_LP_FILTER_CUTOFF, "LP Cutoff" },
	{ 0x49,  TF_LP_FILTER_RESONANCE, "LP Resonance" },

	{ -1, -1, "" }
};
