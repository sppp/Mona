#include "Mona.h"


// Null id.
const ID Mona::NULL_ID = Homeostat::NULL_ID;

// Construct empty network.
Mona::Mona() {
	ClearVars();
	InitParms();
}


void Mona::Init(int sensor_count, int response_count, int need_count) {
	ClearVars();
	InitParms();
	InitNet(sensor_count, response_count, need_count);
}

// Behavior cycle.
RESPONSE Mona::Cycle(Vector<SENSOR>& sensors) {
	// Input sensors.
	ASSERT((int)sensors.GetCount() == sensor_count);
	this->sensors.Clear();

	for (int i = 0; i < sensor_count; i++)
		this->sensors.Add(sensors[i]);

	#ifdef MONA_TRACKING
	// Clear tracking activity.
	ClearTracking();
	#endif
	
	Sense();
	Enable();
	Learn();
	Drive();
	Respond();
	return (response);
}


// Initialize parameters.
void Mona::InitParms() {
	MIN_ENABLEMENT = 0.05;
	ASSERT(MIN_ENABLEMENT > NEARLY_ZERO);
	INITIAL_ENABLEMENT = 0.05;
	ASSERT(INITIAL_ENABLEMENT >= MIN_ENABLEMENT);
	DRIVE_ATTENUATION = 0.0;
	FIRING_STRENGTH_LEARNING_DAMPER = 0.1;
	LEARNING_DECREASE_VELOCITY      = 0.9;
	LEARNING_INCREASE_VELOCITY      = 0.1;
	RESPONSE_RANDOMNESS             = 0.0;
	UTILITY_ASYMPTOTE = 10.0;
	DEFAULT_MAX_LEARNING_EFFECT_EVENT_INTERVAL = 2;
	DEFAULT_NUM_EFFECT_EVENT_INTERVALS         = 3;
	MAX_MEDIATORS      = 200;
	MAX_MEDIATOR_LEVEL = 3;
	MAX_RESPONSE_EQUIPPED_MEDIATOR_LEVEL   = 3;
	MIN_RESPONSE_UNEQUIPPED_MEDIATOR_LEVEL = 3;
	SENSOR_RESOLUTION = 0.0;
	LEARN_MEDIATOR_GOAL_VALUE_MIN_LEVEL = 2;
	LEARN_RECEPTOR_GOAL_VALUE           = false;
	// Initialize effect event intervals.
	InitEffectEventIntervals();
	// Initialize the maximum learning effect event intervals.
	InitMaxLearningEffectEventIntervals();
	// Audit the default effect event intervals.
	ASSERT(AuditDefaultEffectEventIntervals());
}


// Initialize effect event intervals and weights.
void Mona::InitEffectEventIntervals() {
	
	// Create the intervals.
	effect_event_intervals.SetCount(MAX_MEDIATOR_LEVEL + 1);

	for (int i = 0; i <= MAX_MEDIATOR_LEVEL; i++) {
		Vector<int>& intval = effect_event_intervals[i];
		
		intval.SetCount(DEFAULT_NUM_EFFECT_EVENT_INTERVALS);

		for (int j = 0; j < DEFAULT_NUM_EFFECT_EVENT_INTERVALS; j++)
			intval[j] = (int)(pow(2.0, i)) * (j + 1);
	}

	// Initialize the interval weights.
	InitEffectEventIntervalWeights();
}


// Initialize effect event intervals and weights for specific level.
void Mona::InitEffectEventInterval(int level, int numIntervals) {
	Vector<int>& intval = effect_event_intervals[level];
	
	// Create the intervals.
	intval.SetCount(numIntervals);

	for (int i = 0; i < numIntervals; i++)
		intval[i] = (int)(pow(2.0, level)) * (i + 1);

	// Create the interval weights.
	InitEffectEventIntervalWeight(level);
}


// Initialize the effect event interval weights using
// the effect event intervals and a simple inverse proportional rule.
void Mona::InitEffectEventIntervalWeights() {
	effect_event_interval_weights.SetCount(effect_event_intervals.GetCount());

	for (int i = 0; i < effect_event_interval_weights.GetCount(); i++)
		InitEffectEventIntervalWeight(i);
}


// Initialize the effect event interval weights for a specific level.
void Mona::InitEffectEventIntervalWeight(int level) {
	WEIGHT sum;
	sum = 0.0;
	Vector<WEIGHT>& sub = effect_event_interval_weights[level];
	Vector<int>& intval = effect_event_intervals[level];
	
	sub.SetCount(intval.GetCount());

	for (int i = 0; i < sub.GetCount(); i++) {
		sum += (1.0 / (double)intval[i]);
	}
	
	for (int i = 0; i < sub.GetCount(); i++) {
		sub[i] = (1.0 / (double)intval[i]) / sum;
	}
}


// Initialize the maximum learning effect event intervals.
void Mona::InitMaxLearningEffectEventIntervals() {
	max_learning_effect_event_intervals.Clear();

	for (int i = 0; i <= MAX_MEDIATOR_LEVEL; i++) {
		// Response-equipped mediators learn with immediate intervals.
		int j;
		if (i <= MAX_RESPONSE_EQUIPPED_MEDIATOR_LEVEL)
			j = (int)(pow(2.0, i));
		else
			j = (int)(pow(2.0, i)) * DEFAULT_MAX_LEARNING_EFFECT_EVENT_INTERVAL;

		max_learning_effect_event_intervals.Add(j);
	}
}


// Audit default effect event intervals, weights, and learning intervals.
// Return false if invalid interval found.
bool Mona::AuditDefaultEffectEventIntervals() {
	int    i, j;
	WEIGHT sum;

	if (DEFAULT_NUM_EFFECT_EVENT_INTERVALS <= 0)
		return false;

	if (DEFAULT_MAX_LEARNING_EFFECT_EVENT_INTERVAL <= 0)
		return false;

	if (effect_event_intervals.GetCount() != (MAX_MEDIATOR_LEVEL + 1))
		return false;

	if (effect_event_intervals.GetCount() != effect_event_interval_weights.GetCount())
		return false;

	if (max_learning_effect_event_intervals.GetCount() != (MAX_MEDIATOR_LEVEL + 1))
		return false;

	for (i = 0; i <= MAX_MEDIATOR_LEVEL; i++) {
		if ((int)effect_event_intervals[i].GetCount() != DEFAULT_NUM_EFFECT_EVENT_INTERVALS)
			return false;

		for (j = 0; j < effect_event_intervals[i].GetCount(); j++) {
			if (effect_event_intervals[i][j] < (pow(2.0, i)))
				return false;
		}
	}

	for (i = 0; i < effect_event_interval_weights.GetCount(); i++) {
		if ((int)effect_event_interval_weights[i].GetCount() !=
			DEFAULT_NUM_EFFECT_EVENT_INTERVALS)
			return false;

		sum = 0.0;

		for (j = 0; j < effect_event_interval_weights[i].GetCount(); j++)
			sum += effect_event_interval_weights[i][j];

		if (fabs(sum - 1.0) > NEARLY_ZERO)
			return false;
	}

	for (i = 0; i <= MAX_MEDIATOR_LEVEL; i++) {
		if (max_learning_effect_event_intervals[i] < 0)
			return false;
	}

	return true;
}


// Mona initializer.
void Mona::InitNet(int sensor_count, int response_count, int need_count) {
	// Sanity checks.
	ASSERT(sensor_count > 0);
	ASSERT(response_count >= 0);
	ASSERT(need_count > 0);
	ASSERT((MAX_RESPONSE_EQUIPPED_MEDIATOR_LEVEL + 1) >=
		   MIN_RESPONSE_UNEQUIPPED_MEDIATOR_LEVEL);
	ASSERT((int)effect_event_intervals.GetCount() == MAX_MEDIATOR_LEVEL + 1);
	ASSERT((int)max_learning_effect_event_intervals.GetCount() == MAX_MEDIATOR_LEVEL + 1);
	// Clear network.
	Clear();
	// Save parameters.
	this->sensor_count   = sensor_count;
	this->response_count = response_count;
	this->need_count     = need_count;
	
	// Set random seed.
	//random.SRAND(random_seed);
	// Initialize sensors.
	sensors.Clear();
	sensors.SetCount(sensor_count, 0.0);

	// Initialize response apparatus and motor neurons.
	response = 0;
	response_potentials.SetCount(response_count);

	for (int i = 0; i < response_count; i++) {
		response_potentials[i] = 0.0;
		NewMotor(i);
	}

	response_override          = NULL_RESPONSE;
	response_override_potential = -1.0;

	// Create homeostats to manage needs.
	homeostats.SetCount(need_count);
	for(int i = 0; i < homeostats.GetCount(); i++)
		homeostats[i].Init(i, this);

	// Set default maximum cumulative motive.
	max_motive = (MOTIVE)need_count;
	
	// Learning initialization.
	event_clock = 0;
	learning_events.SetCount(MAX_MEDIATOR_LEVEL + 1);
	
	#ifdef MONA_TRACE
	// Tracing.
	trace_sense   = false;
	trace_enable  = false;
	trace_learn   = false;
	trace_drive   = false;
	trace_respond = false;
	#endif
}


// Clear variables.
void Mona::ClearVars() {
	sensor_count                = 0;
	response_count              = 0;
	need_count                  = 0;
	response_override          = NULL_RESPONSE;
	response_override_potential = -1.0;
	id_dispenser               = 0;
}


// Mona destructor.
Mona::~Mona() {
	Clear();
}


// Get need.
NEED Mona::GetNeed(int index) {
	return (homeostats[index].GetNeed());
}


// Set need.
// Value must be in [0,1] interval.
void Mona::SetNeed(int index, NEED value) {
	ASSERT(value >= 0.0 && value <= 1.0);
	homeostats[index].SetNeed(value);
}


// Inflate need to maximum value.
void Mona::InflateNeed(int index) {
	NEED current_need, delta_need, need;
	
	current_need = homeostats[index].GetNeed();
	delta_need   = 1.0 - current_need;
	homeostats[index].SetNeed(current_need + delta_need);

	for (int i = 0; i < mediators.GetCount(); i++) {
		Mediator& mediator = mediators[i];
		
		for(int i = 0; i < mediator.response_enablings.enablings.GetCount(); i++) {
			Enabling& enabling = mediator.response_enablings.enablings[i];
			need     = enabling.needs.Get(index) + delta_need;
			enabling.needs.Set(index, need);
		}
	}

	for (int i = 0; i < learning_events.GetCount(); i++) {
		Array<LearningEvent>& sub = learning_events[i];
		
		for(int j = 0; j < sub.GetCount(); j++) {
			LearningEvent& learning_event = sub[j];
			need = learning_event.needs.Get(index) + delta_need;
			learning_event.needs.Set(index, need);
		}
	}

	for (int i = 0; i < generalization_events.GetCount(); i++) {
		GeneralizationEvent& generalization_event = generalization_events[i];
		need = generalization_event.needs.Get(index) + delta_need;
		generalization_event.needs.Set(index, need);
	}
}


// Set periodic need.
void Mona::SetPeriodicNeed(int index, int frequency, NEED periodic_need) {
	ASSERT(periodic_need >= 0.0 && periodic_need <= 1.0);
	homeostats[index].SetPeriodicNeed(frequency, periodic_need);
}


// Clear periodic need.
void Mona::ClearPeriodicNeed(int index) {
	homeostats[index].ClearPeriodicNeed();
}


// Print need.
/*  void
    Mona::PrintNeed(int index)
    {
    homeostats[index].Print();
    }
*/

// Add a goal for a need.
int Mona::AddGoal(int need_index, Vector<SENSOR>& sensors,
				  SENSOR_MODE sensor_mode, RESPONSE response, NEED goal_value) {
	ASSERT(need_index >= 0 && need_index < homeostats.GetCount());
	return homeostats[need_index].AddGoal(sensors, sensor_mode, response, goal_value);
}


// Add a goal for a need.
int Mona::AddGoal(int need_index, Vector<SENSOR>& sensors,
				  SENSOR_MODE sensor_mode, NEED goal_value) {
	ASSERT(need_index >= 0 && need_index < homeostats.GetCount());
	return homeostats[need_index].AddGoal(sensors, sensor_mode, goal_value);
}


// Find goal for need.
int Mona::FindGoal(int need_index, Vector<SENSOR>& sensors,
				   SENSOR_MODE sensor_mode, RESPONSE response) {
	ASSERT(need_index >= 0 && need_index < homeostats.GetCount());
	return homeostats[need_index].FindGoal(sensors, sensor_mode,
											 response);
}


// Find goal for need.
int Mona::FindGoal(int need_index, Vector<SENSOR>& sensors,
				   SENSOR_MODE sensor_mode) {
	ASSERT(need_index >= 0 && need_index < homeostats.GetCount());
	return homeostats[need_index].FindGoal(sensors, sensor_mode);
}


// Get number of goals for need.
int Mona::GetGoalCount(int need_index) {
	ASSERT(need_index >= 0 && need_index < homeostats.GetCount());
	return homeostats[need_index].goals.GetCount();
}


// Get goal information for need.
bool Mona::GetGoalInfo(int need_index, int goal_index,
					   Vector<SENSOR>& sensors, int& sensor_mode,
					   RESPONSE& response, NEED& goal_value, bool& enabled) {
	ASSERT(need_index >= 0 && need_index < homeostats.GetCount());
	return homeostats[need_index].GetGoalInfo(goal_index, sensors, sensor_mode, response, goal_value, enabled);
}


// Is goal for need enabled?
bool Mona::IsGoalEnabled(int need_index, int goal_index) {
	ASSERT(need_index >= 0 && need_index < homeostats.GetCount());
	return homeostats[need_index].IsGoalEnabled(goal_index);
}


// Enable goal for need.
bool Mona::EnableGoal(int need_index, int goal_index) {
	ASSERT(need_index >= 0 && need_index < homeostats.GetCount());
	return homeostats[need_index].EnableGoal(goal_index);
}


// Disable goal for need.
bool Mona::DisableGoal(int need_index, int goal_index) {
	ASSERT(need_index >= 0 && need_index < homeostats.GetCount());
	return homeostats[need_index].DisableGoal(goal_index);
}


// Remove goal for need.
bool Mona::RemoveGoal(int need_index, int goal_index) {
	ASSERT(need_index >= 0 && need_index < homeostats.GetCount());
	return homeostats[need_index].RemoveGoal(goal_index);
}


// Create receptor and add to network.
Receptor& Mona::NewReceptor(Vector<SENSOR>& centroid, SENSOR_MODE sensor_mode) {
	Receptor& r = receptors.Add();
	r.Init(centroid, sensor_mode, this);
	
	r.id = id_dispenser;
	id_dispenser++;
	r.creation_time = event_clock;
	
	Vector<SENSOR> sensors;
	sensors.SetCount(r.centroid.GetCount());
	for (int i = 0; i < r.centroid.GetCount(); i++)
		sensors[i] = r.centroid[i];

	sensor_centroids[sensor_mode].Insert(sensors, r);
	
	return r;
}


// Create motor and add to network.
Motor& Mona::NewMotor(RESPONSE response) {
	Motor& m = motors.Add();
	m.Init(response, this);
	m.id = id_dispenser;
	id_dispenser++;
	m.creation_time = event_clock;
	return m;
}



// Create mediator and add to network.
Mediator& Mona::AddMediator(ENABLEMENT enablement) {
	Mediator& m = mediators.Add();
	m.Init(enablement, this);
	m.id = id_dispenser;
	id_dispenser++;
	m.creation_time = event_clock;
	return m;
}


// Remove neuron from network.
void Mona::DeleteNeuron(Mediator& neuron) {
	// Delete parents.
	while (neuron.notify_list.GetCount() > 0) {
		Notify& notify = neuron.notify_list[0];
		ASSERT(notify.mediator);
		int c = neuron.notify_list.GetCount();
		DeleteNeuron(*notify.mediator);
		ASSERT(c > neuron.notify_list.GetCount());
		/*if (c == neuron.notify_list.GetCount())
			neuron.notify_list.Remove(0);*/
	}
	
	ASSERT(neuron.type == MEDIATOR);
	Mediator& mediator = neuron;
	int space = mediator.level + 1;
		
	if (space < learning_events.GetCount()) {
		Array<LearningEvent>& sub = learning_events[space];
		
		for(int j = 0; j < sub.GetCount();) {
			LearningEvent& learning_event = sub[j];

			if (learning_event.neuron == &neuron) {
				sub.Remove(j);
			}
			else
				j++;
		}
	}
	
	for(int i = 0; i < mediators.GetCount(); i++) {
		if (&mediators[i] == &mediator) {
			mediators.Remove(i);
			return;
		}
	}
	
	Panic("Deleting mediator failed");
}

void Mona::DeleteNeuron(Receptor& neuron) {
	// Delete parents.
	while (neuron.notify_list.GetCount() > 0) {
		Notify& notify = neuron.notify_list[0];
		ASSERT(notify.mediator);
		int c = neuron.notify_list.GetCount();
		DeleteNeuron(*notify.mediator);
		ASSERT(c > neuron.notify_list.GetCount());
		/*if (c == neuron.notify_list.GetCount())
			neuron.notify_list.Remove(0);*/
	}
	
	int space = 0;
	
	if (space < learning_events.GetCount()) {
		Array<LearningEvent>& sub = learning_events[space];
		
		for(int j = 0; j < sub.GetCount();) {
			LearningEvent& learning_event = sub[j];

			if (learning_event.neuron == &neuron) {
				sub.Remove(j);
			}
			else
				j++;
		}
	}
	
	Receptor& receptor = neuron;
	
	for (int i = 0; i < receptor.sub_sensor_modes.GetCount(); i++) {
		Receptor& ref_receptor = *receptor.sub_sensor_modes[i];

		for (int j = 0; j < ref_receptor.super_sensor_modes.GetCount(); j++) {
			if (ref_receptor.super_sensor_modes[j] == &receptor) {
				ref_receptor.super_sensor_modes.Remove(j);
				break;
			}
		}
	}

	for (int i = 0; i < receptor.super_sensor_modes.GetCount(); i++) {
		Receptor& ref_receptor = *receptor.super_sensor_modes[i];

		for (int j = 0; j < ref_receptor.sub_sensor_modes.GetCount(); j++) {
			if (ref_receptor.sub_sensor_modes[j] == &receptor) {
				ref_receptor.sub_sensor_modes.Remove(j);
				break;
			}
		}
	}

	for (int i = 0; i < homeostats.GetCount(); i++)
		homeostats[i].RemoveNeuron(&neuron);

	if (sensor_centroids.GetCount() > 0)
		sensor_centroids[receptor.sensor_mode].Remove(receptor.centroid);
	
	for (int i = 0; i < receptors.GetCount(); i++) {
		if (&receptors[i] == &receptor) {
			receptors.Remove(i);
			return;
		}
	}
	
	Panic("Deleting receptor failed");
}

/*void Mona::DeleteNeuron(Neuron& neuron) {

	// Delete parents.
	while (neuron.notify_list.GetCount() > 0) {
		Notify& notify = neuron.notify_list[0];
		DeleteNeuron(*notify.mediator);
		
		// Conversion addition:
		//notify.mediator = NULL;
		ASSERT(notify.mediator == NULL); // somebody have to clean this
	}

	// Remove from learning space.
	int space;
	if (neuron.type != MEDIATOR)
		space = 0;
	else {
		Mediator& mediator = dynamic_cast<Mediator&>(neuron);
		space = mediator.level + 1;
	}

	if (space < learning_events.GetCount()) {
		Array<LearningEvent>& sub = learning_events[space];
		
		for(int j = 0; j < sub.GetCount();) {
			LearningEvent& learning_event = sub[j];

			if (learning_event.neuron == &neuron) {
				sub.Remove(j);
			}
			else
				j++;
		}
	}

	// Delete neuron.
	if (neuron.type == RECEPTOR) {
		Receptor& receptor = dynamic_cast<Receptor&>(neuron);
		
		for (int i = 0; i < receptors.GetCount(); i++) {
			if (&receptors[i] == &receptor) {
				receptors.Remove(i);
				break;
			}
		}

		for (int i = 0; i < receptor.sub_sensor_modes.GetCount(); i++) {
			Receptor& ref_receptor = *receptor.sub_sensor_modes[i];

			for (int j = 0; j < ref_receptor.super_sensor_modes.GetCount(); j++) {
				if (ref_receptor.super_sensor_modes[j] == &receptor) {
					ref_receptor.super_sensor_modes.Remove(j);
					break;
				}
			}
		}

		for (int i = 0; i < receptor.super_sensor_modes.GetCount(); i++) {
			Receptor& ref_receptor = *receptor.super_sensor_modes[i];

			for (int j = 0; j < ref_receptor.sub_sensor_modes.GetCount(); j++) {
				if (ref_receptor.sub_sensor_modes[j] == &receptor) {
					ref_receptor.sub_sensor_modes.Remove(j);
					break;
				}
			}
		}

		for (int i = 0; i < homeostats.GetCount(); i++)
			homeostats[i].RemoveNeuron(&neuron);

		if (sensor_centroids.GetCount() > 0)
			sensor_centroids[receptor.sensor_mode]->Remove((void*) & (receptor.centroid));
		#error wtf
		delete receptor;
	}
	else if (neuron.type == MOTOR) {
		Motor& motor = dynamic_cast<Motor&>(neuron);
		
		for (int i = 0; i < motors.GetCount(); i++) {
			if (motors[i] == motor) {
				motors.Remove(motors.Begin() + i);
				break;
			}
		}

		for (int i = 0; i < homeostats.GetCount(); i++)
			homeostats[i].RemoveNeuron(&neuron);
		
		#error wtf
		delete (Motor*)neuron;
	}
	else if (neuron.type == MEDIATOR) {
		mediators.Remove(mediator);
		break;
	}
	else Panic("Invalid type");
}*/


// Get mediator with worst utility.
// Skip instinct mediators and those with instinct parents.
Mediator*
Mona::GetWorstMediator(int min_level) {
	//Mediator* mediator;
	//Vector<Mediator*>::Iterator mediator_iter;
	Vector<Mediator*>         worst_mediators;
	UTILITY utility, worst_utility;
	worst_utility = 0.0;
	
	for(int i = 0; i < mediators.GetCount(); i++) {
		Mediator& mediator = mediators[i];

		if (mediator.level < min_level)
			continue;

		if (mediator.instinct || mediator.HasInnerInstinct())
			continue;

		utility = mediator.GetEffectiveUtility();

		if ((worst_mediators.GetCount() == 0) || (utility < worst_utility)) {
			worst_mediators.Clear();
			worst_mediators.Add(&mediator);
			worst_utility = utility;
		}
		else if ((utility - worst_utility) <= NEARLY_ZERO)
			worst_mediators.Add(&mediator);
	}

	if (worst_mediators.GetCount() == 0)
		return NULL;
	else {
		// Return a random choice of worst.
		return worst_mediators[Random((int)worst_mediators.GetCount())];
	}
}


// Get mediator with best utility.
Mediator*
Mona::GetBestMediator(int min_level) {
	Vector<Mediator*>         best_mediators;
	UTILITY utility, best_utility;
	best_utility = 0.0;
	
	
	for(int i = 0; i < mediators.GetCount(); i++) {
		Mediator& mediator = mediators[i];

		if (mediator.level < min_level)
			continue;

		utility = mediator.GetEffectiveUtility();

		if ((best_mediators.GetCount() == 0) || (utility > best_utility)) {
			best_mediators.Clear();
			best_mediators.Add(&mediator);
			best_utility = utility;
		}
		else if ((best_utility - utility) <= NEARLY_ZERO)
			best_mediators.Add(&mediator);
	}

	if (best_mediators.GetCount() == 0)
		return NULL;
	else {
		// Return a random choice.
		return (best_mediators[Random((int)best_mediators.GetCount())]);
	}
}


// Load network from a file.
void Mona::Load(String filename) {
	LoadFromFile(*this, filename);
}

// Save network to file.
void Mona::Store(String filename) {
	StoreToFile(*this, filename);
}

inline int64 Max64(int64 a, int64 b) {
	if (a > b) return a;
	else return b;
}

void Mona::Serialize(Stream& fp) {
	fp % MIN_ENABLEMENT % INITIAL_ENABLEMENT % DRIVE_ATTENUATION;
	fp % FIRING_STRENGTH_LEARNING_DAMPER % LEARNING_DECREASE_VELOCITY % LEARNING_INCREASE_VELOCITY;
	fp % RESPONSE_RANDOMNESS % UTILITY_ASYMPTOTE % DEFAULT_MAX_LEARNING_EFFECT_EVENT_INTERVAL;
	fp % DEFAULT_NUM_EFFECT_EVENT_INTERVALS % MAX_MEDIATORS % MAX_MEDIATOR_LEVEL;
	fp % MAX_RESPONSE_EQUIPPED_MEDIATOR_LEVEL % MIN_RESPONSE_UNEQUIPPED_MEDIATOR_LEVEL % SENSOR_RESOLUTION;
	fp % LEARN_MEDIATOR_GOAL_VALUE_MIN_LEVEL % LEARN_RECEPTOR_GOAL_VALUE % effect_event_intervals;
	fp % effect_event_interval_weights % max_learning_effect_event_intervals % sensor_count;
	fp % response_count % need_count;
	
	InitNet(sensor_count, response_count, need_count);
	
	fp % sensor_modes % sensors % response;
	fp % event_clock % learning_events % receptors;
	fp % motors % mediators % homeostats;
	
	
	if (fp.IsLoading()) {
		id_dispenser = 0;
		
		for (int i = 0; i < receptors.GetCount(); i++) {
			Receptor& r = receptors[i];
			id_dispenser = Max64(id_dispenser, r.id+1);
			r.Connect(this);
		}
		
		for(int i = 0; i < motors.GetCount(); i++) {
			Motor& m = motors[i];
			id_dispenser = Max64(id_dispenser, m.id+1);
			m.Neuron::Connect(this);
		}
		
		for(int i = 0; i < mediators.GetCount(); i++) {
			Mediator& m = mediators[i];
			id_dispenser = Max64(id_dispenser, m.id+1);
			m.Neuron::Connect(this);
		}
		
		// Resolve neuron addresses using id.
		for (int i = 0; i < receptors.GetCount(); i++) {
			Receptor& r = receptors[i];
			for (int j = 0; j < r.sub_sensor_modes.GetCount(); j++) {
				ReceptorID& id = r.sub_sensor_modes[j];
				ASSERT(id.FindFrom(this));
			}
			for (int j = 0; j < r.super_sensor_modes.GetCount(); j++) {
				ReceptorID& id = r.super_sensor_modes[j];
				ASSERT(id.FindFrom(this));
			}
			for (int j = 0; j < r.notify_list.GetCount(); j++) {
				MediatorID& id = r.notify_list[j].mediator;
				ASSERT(id.FindFrom(this));
			}
		}

		for (int i = 0; i < motors.GetCount(); i++) {
			Motor& motor = motors[i];

			for (int j = 0; j < motor.notify_list.GetCount(); j++) {
				Notify& notify = motor.notify_list[j];
				ASSERT(notify.mediator.FindFrom(this));
			}
		}
		
		for(int i = 0; i < mediators.GetCount(); i++) {
			Mediator& mediator = mediators[i];
			ASSERT(mediator.cause.FindFrom(this));
			ASSERT(mediator.effect.FindFrom(this));
			mediator.response.FindFrom(this); // this is allowed to fail in original
			
			for (int j = 0; j < mediator.notify_list.GetCount(); j++) {
				Notify& notify = mediator.notify_list[j];
				ASSERT(notify.mediator.FindFrom(this));
			}
		}

		for (int i = 0; i < learning_events.GetCount(); i++) {
			Array<LearningEvent>& sub = learning_events[i];
			for(int j = 0; j < sub.GetCount(); j++) {
				LearningEvent& learning_event = sub[j];
				NeuronID& id = learning_event.neuron;
				ASSERT(id.FindFrom(this));
			}
		}
		
		int count;
		fp % count;
		sensor_centroids.SetCount(count);
		for(int i = 0; i < sensor_centroids.GetCount(); i++) {
			RDTree& rd = sensor_centroids[i];
			
			rd.Connect(Receptor::PatternDistance, Receptor::DeletePattern);
			rd.Load(fp, *this, Receptor::LoadPattern, Receptor::LoadClient);
		}
	}
	
	// Storing
	else {
		int count = sensor_centroids.GetCount();
		fp % count;
		for(int i = 0; i < sensor_centroids.GetCount(); i++) {
			RDTree& rd = sensor_centroids[i];
			rd.Store(fp, Receptor::StorePattern, Receptor::StoreClient);
		}
	}
}


// Find neuron by id.
Neuron* Mona::FindByID(int id) {
	
	for (int i = 0; i < receptors.GetCount(); i++) {
		Receptor& receptor = receptors[i];

		if (receptor.id == id)
			return &receptor;
	}

	for (int i = 0; i < motors.GetCount(); i++) {
		Motor& motor = motors[i];

		if (motor.id == id)
			return &motor;
	}

	for (int i = 0; i < mediators.GetCount(); i++) {
		Mediator& mediator = mediators[i];

		if (mediator.id == id)
			return &mediator;
	}

	return NULL;
}

/*
// When changing format increment FORMAT in mona.h
bool Mona::Store(Stream& fp) {
	int           i, j, k;
	int64          t;
	double        d;
	int           format;
	LearningEvent* learning_event;
	//Vector<LearningEvent*>::Iterator learning_event_iter;
	Receptor* receptor;
	Motor*    motor;
	Mediator* mediator;
	//Vector<Mediator*>::Iterator mediator_iter;
	
	// Save format, including searchable string.
	format = FORMAT;
	FWRITE_INT(&format, fp);
	char buf[40];
	sprintf(buf, "%s(#) Mona format %d", "@", FORMAT);
	FWRITE_STRING(buf, 40, fp);
	FWRITE_DOUBLE(&MIN_ENABLEMENT, fp);
	FWRITE_DOUBLE(&INITIAL_ENABLEMENT, fp);
	FWRITE_DOUBLE(&DRIVE_ATTENUATION, fp);
	FWRITE_DOUBLE(&FIRING_STRENGTH_LEARNING_DAMPER, fp);
	FWRITE_DOUBLE(&LEARNING_DECREASE_VELOCITY, fp);
	FWRITE_DOUBLE(&LEARNING_INCREASE_VELOCITY, fp);
	FWRITE_DOUBLE(&RESPONSE_RANDOMNESS, fp);
	FWRITE_DOUBLE(&UTILITY_ASYMPTOTE, fp);
	FWRITE_INT(&DEFAULT_MAX_LEARNING_EFFECT_EVENT_INTERVAL, fp);
	FWRITE_INT(&DEFAULT_NUM_EFFECT_EVENT_INTERVALS, fp);
	FWRITE_INT(&MAX_MEDIATORS, fp);
	FWRITE_INT(&MAX_MEDIATOR_LEVEL, fp);
	FWRITE_INT(&MAX_RESPONSE_EQUIPPED_MEDIATOR_LEVEL, fp);
	FWRITE_INT(&MIN_RESPONSE_UNEQUIPPED_MEDIATOR_LEVEL, fp);
	FWRITE_FLOAT(&SENSOR_RESOLUTION, fp);
	FWRITE_INT(&LEARN_MEDIATOR_GOAL_VALUE_MIN_LEVEL, fp);
	FWRITE_BOOL(&LEARN_RECEPTOR_GOAL_VALUE, fp);

	for (i = 0; i < effect_event_intervals.GetCount(); i++) {
		k = (int)effect_event_intervals[i].GetCount();
		FWRITE_INT(&k, fp);

		for (j = 0; j < k; j++) {
			t = effect_event_intervals[i][j];
			FWRITE_LONG_LONG(&t, fp);
		}
	}

	for (i = 0; i < effect_event_interval_weights.GetCount(); i++) {
		for (j = 0; j < effect_event_interval_weights[i].GetCount(); j++) {
			d = effect_event_interval_weights[i][j];
			FWRITE_DOUBLE(&d, fp);
		}
	}

	for (i = 0; i < max_learning_effect_event_intervals.GetCount(); i++) {
		t = max_learning_effect_event_intervals[i];
		FWRITE_LONG_LONG(&t, fp);
	}

	FWRITE_INT(&sensor_count, fp);
	FWRITE_INT(&response_count, fp);
	FWRITE_INT(&need_count, fp);
	FWRITE_LONG(&random_seed, fp);
	j = (int)sensor_modes.GetCount();
	FWRITE_INT(&j, fp);

	for (i = 0; i < j; i++)
		sensor_modes[i]->Store(fp);

	random.RAND_SAVE(fp);

	for (i = 0; i < sensors.GetCount(); i++)
		FWRITE_FLOAT(&sensors[i], fp);

	FWRITE_INT(&response, fp);
	FWRITE_LONG_LONG(&event_clock, fp);

	for (i = 0; i < learning_events.GetCount(); i++) {
		j = (int)learning_events[i].GetCount();
		FWRITE_INT(&j, fp);

		for (learning_event_iter = learning_events[i].Begin();
			 learning_event_iter != learning_events[i].End(); learning_event_iter++) {
			learning_event = *learning_event_iter;
			learning_event.Store(fp);
		}
	}

	i = (int)receptors.GetCount();
	FWRITE_INT(&i, fp);

	for (i = 0; i < receptors.GetCount(); i++) {
		receptor = receptors[i];
		receptor.Store(fp);
	}

	for (i = 0; i < motors.GetCount(); i++) {
		motor = motors[i];
		motor.Store(fp);
	}

	i = (int)mediators.GetCount();
	FWRITE_INT(&i, fp);

	for (mediator_iter = mediators.Begin();
		 mediator_iter != mediators.End(); mediator_iter++) {
		mediator = *mediator_iter;
		mediator.Store(fp);
	}

	for (i = 0; i < need_count; i++)
		homeostats[i].Store(fp);

	j = (int)sensor_centroids.GetCount();
	FWRITE_INT(&j, fp);

	for (i = 0; i < j; i++) {
		sensor_centroids[i]->Store(fp, Receptor::store_patternern,
								   Receptor::store_client);
	}

	return true;
}*/


// Clear the network.
void Mona::Clear() {
	/*int      i;
	Receptor* receptor;
	Motor*    motor;
	//Vector<Mediator*>::Iterator      mediator_iter;
	LearningEvent*                   learning_event;
	//Vector<LearningEvent*>::Iterator learning_event_iter;
	*/
	sensors.Clear();
	sensor_modes.Clear();
	sensor_centroids.Clear();

	while (receptors.GetCount() > 0) {
		Receptor& receptor = receptors[0];
		DeleteNeuron(receptor);
	}
	
	receptors.Clear();
	response_potentials.Clear();
	motors.Clear();
	homeostats.Clear();
	learning_events.Clear();
	
	ClearVars();
}

/*
    // Print network.
    bool
    Mona::Print(String filename)
    {
    if (filename == NULL)
    {
      Print();
    }
    else
    {
      Stream& fp = fopen(filename, "w");
      if (fp == NULL)
      {
         return false;
      }
      Print(fp);
      fclose(fp);
    }
    return true;
    }


    #ifdef MONA_TRACKING
    void
    Mona::Print(FILE *out)
    {
    Print((TRACKING_FLAGS)0, out);
    }


    void
    Mona::Print(TRACKING_FLAGS tracking, FILE *out)
    {
    Print(tracking, false, out);
    }


    #else
    void
    Mona::Print(FILE *out)
    {
    Print(false, out);
    }


    #endif

    #ifdef MONA_TRACKING
    void
    Mona::PrintBrief(FILE *out)
    {
    Print((TRACKING_FLAGS)0, true, out);
    }


    void
    Mona::PrintBrief(TRACKING_FLAGS tracking, FILE *out)
    {
    Print(tracking, true, out);
    }


    #else
    void
    Mona::PrintBrief(FILE *out)
    {
    Print(true, out);
    }


    #endif

    #ifdef MONA_TRACKING
    void
    Mona::Print(TRACKING_FLAGS tracking, bool brief, FILE *out)
    #else
    void
    Mona::Print(bool brief, FILE *out)
    #endif
    {
    int      i;
    Receptor *receptor;
    Motor    *motor;
    Mediator *mediator;

    Vector<Mediator *>::Iterator mediator_iter;
    #ifdef MONA_TRACKING
    int          p, q, r, s, t;
    Motor        *driveMotor;
    Neuron       *neuron;
    Vector<bool> pathDone;
    #endif

    fprintf(out, "<network>\n");
    fprintf(out, "<format>%d</format>\n", FORMAT);
    PrintParms(out);
    fprintf(out, "<sensor_count>%d</sensor_count>\n", sensor_count);
    fprintf(out, "<sensor_modes>");
    for (i = 0; i < sensor_modes.GetCount(); i++)
    {
      sensor_modes[i]->Print(out);
    }
    fprintf(out, "</sensor_modes>\n");
    fprintf(out, "<response_count>%d</response_count>\n", response_count);
    fprintf(out, "<need_count>%d</need_count>\n", need_count);
    fprintf(out, "<random_seed>%lu</random_seed>\n", random_seed);
    fprintf(out, "<needs>\n");
    for (i = 0; i < need_count; i++)
    {
      homeostats[i].Print(out);
      fprintf(out, "\n");
    }
    fprintf(out, "</needs>\n");
    fprintf(out, "<event_clock>%llu</event_clock>\n", event_clock);
    #ifdef MONA_TRACKING
    if (tracking & TRACK_DRIVE)
    {
      for (i = 0; i < motors.GetCount(); i++)
      {
         driveMotor = motors[i];
         if (driveMotor->tracker.fire)
         {
            break;
         }
      }
      if (i == (int)motors.GetCount())
      {
         driveMotor = NULL;
         q          = 1;
      }
      else
      {
         q = (int)driveMotor->tracker.motive_paths.GetCount();
         for (p = 0; p < q; p++)
         {
            pathDone.Add(false);
         }
      }
    }
    else
    {
      driveMotor = NULL;
      q          = 1;
    }
    for (p = 0; p < q; p++)
    {
      if (driveMotor != NULL)
      {
         for (r = 0, t = -1, s = (int)driveMotor->tracker.motive_paths.GetCount(); r < s; r++)
         {
            if (pathDone[r])
            {
               continue;
            }
            if ((t == -1) ||
                (driveMotor->tracker.motive_paths[r].motive >= driveMotor->tracker.motive_paths[t].motive))
            {
               t = r;
            }
         }
         pathDone[t] = true;
         fprintf(out, "<drive_network motive=%f>\n", driveMotor->tracker.motive_paths[t].motive);
         for (r = 0, s = (int)driveMotor->tracker.motive_paths[t].drivers.GetCount(); r < s; r++)
         {
            neuron = driveMotor->tracker.motive_paths[t].drivers[r].neuron;
            neuron->tracker.drive  = true;
            neuron->tracker.motive = driveMotor->tracker.motive_paths[t].drivers[r].motive;
         }
      }
    #endif
    fprintf(out, "<receptors>\n");
    for (i = 0; i < receptors.GetCount(); i++)
    {
      receptor = receptors[i];
    #ifdef MONA_TRACKING
      if (((tracking & TRACK_FIRE) && receptor.tracker.fire) ||
          ((tracking & TRACK_ENABLE) && receptor.tracker.enable) ||
          ((tracking & TRACK_DRIVE) && receptor.tracker.drive))
      {
         receptor.Print(tracking, out);
         fprintf(out, "\n");
      }
    #else
      receptor.Print(out);
      fprintf(out, "\n");
    #endif
    }
    fprintf(out, "</receptors>\n");
    fprintf(out, "<motors>\n");
    for (i = 0; i < motors.GetCount(); i++)
    {
      motor = motors[i];
    #ifdef MONA_TRACKING
      if (((tracking & TRACK_FIRE) && motor.tracker.fire) ||
          ((tracking & TRACK_ENABLE) && motor.tracker.enable) ||
          ((tracking & TRACK_DRIVE) && motor.tracker.drive))
      {
         motor.Print(tracking, out);
         fprintf(out, "\n");
      }
    #else
      motor.Print(out);
      fprintf(out, "\n");
    #endif
    }
    fprintf(out, "</motors>\n");
    fprintf(out, "<mediators>\n");
    for (mediator_iter = mediators.Begin();
        mediator_iter != mediators.End(); mediator_iter++)
    {
      mediator = *mediator_iter;
    #ifdef MONA_TRACKING
      if (((tracking & TRACK_FIRE) && mediator.tracker.fire) ||
          ((tracking & TRACK_ENABLE) && mediator.tracker.enable) ||
          ((tracking & TRACK_DRIVE) && mediator.tracker.drive))
      {
         mediator.Print(tracking, brief, 0, out);
      }
    #else
      mediator.Print(brief, 0, out);
    #endif
    }
    fprintf(out, "</mediators>\n");
    #ifdef MONA_TRACKING
    if (driveMotor != NULL)
    {
      fprintf(out, "</drive_network>\n");
      for (r = 0, s = (int)driveMotor->tracker.motive_paths[t].drivers.GetCount(); r < s; r++)
      {
         neuron = driveMotor->tracker.motive_paths[t].drivers[r].neuron;
         neuron->tracker.drive  = false;
         neuron->tracker.motive = 0.0;
      }
    }
    }
    #endif





    fprintf(out, "</network>\n");
    fflush(out);
    }


    // Print parameters.
    void
    Mona::PrintParms(FILE *out)
    {
    
    fprintf(out, "<parameters>\n");
    fprintf(out, "<parameter>MIN_ENABLEMENT</parameter><value>%f</value>\n", MIN_ENABLEMENT);
    fprintf(out, "<parameter>INITIAL_ENABLEMENT</parameter><value>%f</value>\n", INITIAL_ENABLEMENT);
    fprintf(out, "<parameter>DRIVE_ATTENUATION</parameter><value>%f</value>\n", DRIVE_ATTENUATION);
    fprintf(out, "<parameter>FIRING_STRENGTH_LEARNING_DAMPER</parameter><value>%f</value>\n", FIRING_STRENGTH_LEARNING_DAMPER);
    fprintf(out, "<parameter>LEARNING_DECREASE_VELOCITY</parameter><value>%f</value>\n", LEARNING_DECREASE_VELOCITY);
    fprintf(out, "<parameter>LEARNING_INCREASE_VELOCITY</parameter><value>%f</value>\n", LEARNING_INCREASE_VELOCITY);
    fprintf(out, "<parameter>RESPONSE_RANDOMNESS</parameter><value>%f</value>\n", RESPONSE_RANDOMNESS);
    fprintf(out, "<parameter>UTILITY_ASYMPTOTE</parameter><value>%f</value>\n", UTILITY_ASYMPTOTE);
    fprintf(out, "<parameter>DEFAULT_MAX_LEARNING_EFFECT_EVENT_INTERVAL</parameter><value>%d</value>\n", DEFAULT_MAX_LEARNING_EFFECT_EVENT_INTERVAL);
    fprintf(out, "<parameter>DEFAULT_NUM_EFFECT_EVENT_INTERVALS</parameter><value>%d</value>\n", DEFAULT_NUM_EFFECT_EVENT_INTERVALS);
    fprintf(out, "<parameter>MAX_MEDIATORS</parameter><value>%d</value>\n", MAX_MEDIATORS);
    fprintf(out, "<parameter>MAX_MEDIATOR_LEVEL</parameter><value>%d</value>\n", MAX_MEDIATOR_LEVEL);
    fprintf(out, "<parameter>MAX_RESPONSE_EQUIPPED_MEDIATOR_LEVEL</parameter><value>%d</value>\n", MAX_RESPONSE_EQUIPPED_MEDIATOR_LEVEL);
    fprintf(out, "<parameter>MIN_RESPONSE_UNEQUIPPED_MEDIATOR_LEVEL</parameter><value>%d</value>\n", MIN_RESPONSE_UNEQUIPPED_MEDIATOR_LEVEL);
    fprintf(out, "<parameter>SENSOR_RESOLUTION</parameter><value>%f</value>\n", SENSOR_RESOLUTION);
    fprintf(out, "<parameter>LEARN_MEDIATOR_GOAL_VALUE_MIN_LEVEL</parameter><value>%d</value>\n", LEARN_MEDIATOR_GOAL_VALUE_MIN_LEVEL);
    if (LEARN_RECEPTOR_GOAL_VALUE)
    {
      fprintf(out, "<parameter>LEARN_RECEPTOR_GOAL_VALUE</parameter><value>true</value>\n");
    }
    else
    {
      fprintf(out, "<parameter>LEARN_RECEPTOR_GOAL_VALUE</parameter><value>false</value>\n");
    }
    fprintf(out, "<effect_event_intervals>\n");
    for (i = 0; i < effect_event_intervals.GetCount(); i++)
    {
      fprintf(out, "<intervals><level>%d</level>", i);
      for (j = 0; j < effect_event_intervals[i].GetCount(); j++)
      {
         fprintf(out, "<time>%llu</time>", effect_event_intervals[i][j]);
      }
      fprintf(out, "</intervals>\n");
    }
    fprintf(out, "</effect_event_intervals>\n");
    fprintf(out, "<effect_event_interval_weights>\n");
    for (i = 0; i < effect_event_interval_weights.GetCount(); i++)
    {
      fprintf(out, "<weights><level>%d</level>", i);
      for (j = 0; j < effect_event_interval_weights[i].GetCount(); j++)
      {
         fprintf(out, "<weight>%f</weight>", effect_event_interval_weights[i][j]);
      }
      fprintf(out, "</weights>\n");
    }
    fprintf(out, "</effect_event_interval_weights>\n");
    fprintf(out, "<maximum_learning_effect_event_intervals>\n");
    for (i = 0; i <= MAX_MEDIATOR_LEVEL; i++)
    {
      fprintf(out, "<interval><level>%d</level><time>%llu</time></interval>\n",
              i, max_learning_effect_event_intervals[i]);
    }
    fprintf(out, "</maximum_learning_effect_event_intervals>\n");
    fprintf(out, "</parameters>\n");
    fflush(out);
    }
*/

#ifdef MONA_TRACKING
// Clear tracking activity.
void Mona::ClearTracking() {
	for (i = 0; i < receptors.GetCount(); i++) {
		receptor = receptors[i];
		receptor.tracker.Clear();
	}

	for (i = 0; i < motors.GetCount(); i++) {
		motor = motors[i];
		motor.tracker.Clear();
	}

	for (mediator_iter = mediators.Begin();
		 mediator_iter != mediators.End(); mediator_iter++) {
		mediator = *mediator_iter;
		mediator.tracker.Clear();
	}
}


#endif
