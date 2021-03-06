#ifndef _MonaCtrl_TrainingGraph_h_
#define _MonaCtrl_TrainingGraph_h_


class TrainingGraph : public ParentCtrl {
	
protected:
	friend class LayerView;
	
	/*Session* ses;
	PlotCtrl plotter;
	int average_size;
	int last_steps;
	int interval;
	int limit;
	int mode;*/
	
	enum {MODE_LOSS, MODE_REWARD};
	
public:
	typedef TrainingGraph CLASSNAME;
	TrainingGraph();
	/*
	void SetSession(Session& ses);
	void SetModeLoss() {mode = MODE_LOSS; plotter.data[0].SetTitle("Loss");}
	void SetModeReward() {mode = MODE_REWARD; plotter.data[0].SetTitle("Reward");}
	void SetAverage(int size) {average_size = size;}
	void SetInterval(int period) {interval = period;}
	void SetLimit(int size) {limit = size;}
	
	void StepInterval(int num_steps);
	void RefreshData();
	void AddValue(double value);
	void AddValue();
	void Clear();*/
};

#endif
