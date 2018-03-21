
GSHARE_BP::GSHARE_BP(const Params *params)
{
    //Set up the array of counters for the local predictor
    localCtrs.resize(localPredictorSize);

    for (int i = 0; i < localPredictorSize; ++i)
        localCtrs[i].setBits(localCtrBits);

    //Clear the global history
    globalHistory = 0;

    //Set up historyRegisterMask
    historyRegisterMask = mask(globalHistoryBits);

    // Set thresholds for the three predictors' counters
    // This is equivalent to (2^(Ctr))/2 - 1
    localThreshold  = (ULL(1) << (localCtrBits  - 1)) - 1;
	
}

bool
GSHARE_BP::lookup(Addr branch_addr, void * &bp_history)
{
    bool local_prediction;
    unsigned gshare;
	
	// gshare hash function branch_addr XOR globalHistory 
	// ref: Combining Branch Predictors - Scott McFarling
	gshare =  ((branch_addr >> instShiftAmt) ^ globalHistory) & historyRegisterMask;
	assert(gshare < localPredictorSize);
		
    local_prediction = localCtrs[gshare].read() > localThreshold;
		
	if (local_prediction) { //from updateGlobalHistTaken();
		globalHistory = (globalHistory << 1) | 1;
		globalHistory = globalHistory & historyRegisterMask;
		return true;
	} else {  //from updateGlobalHistNotTaken();
		globalHistory = (globalHistory << 1);
		globalHistory = globalHistory & historyRegisterMask;
		return false;
	}
}

void
GSHARE_BP::update(Addr branch_addr, bool taken, void *bp_history, bool squashed) {
	unsigned gshare;

	// gshare hash function branch_addr XOR globalHistory
	gshare =  ((branch_addr >> instShiftAmt) ^ globalHistory) & historyRegisterMask;
	assert(gshare < localPredictorSize);
	
	if (taken) {
		 // Update the counters
		localCtrs[gshare].increment();
	} else {
		localCtrs[gshare].decrement();
	}
	
	if (squashed) {	
		if (taken) {
			globalHistory = (globalHistory << 1) | 1;
			globalHistory = globalHistory & historyRegisterMask;
		 } else {
			globalHistory = (globalHistory << 1);
			globalHistory = globalHistory & historyRegisterMask;
		 }
	}
}
