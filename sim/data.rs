#[derive(Default)]
pub struct GameState {
	pub friendly_bot_1: BotState,
	pub friendly_bot_2: BotState,
	pub enemy_bot_1: BotState,
	pub enemy_bot_2: BotState,
	pub ball: BallState,
	pub score: ScoreState
}

#[derive(Default)]
pub struct Action {
	pub wheel_accels: Vec<f64>,
	pub dribber_accel: f64,
	pub kicker_accel: f64,
}

#[derive(Default)]
pub struct ViewState {
	pub lidar_readings: Vec<f64>,
	pub line_sensor_readings: Vec<u8>,
	pub relative_ball_pos: Vec<f64>,
	pub relative_ball_vel: Vec<f64>,
	pub relative_self_goal_pos: Vec<f64>,
	pub relative_target_goal_pos: Vec<f64>,
}

#[derive(Default)]
pub struct BotState {
	pub position: Vec<f64>,
	pub velocity: Vec<f64>,
	pub orientation: Vec<f64>,
	pub spin: Vec<f64>,
	pub wheel_states: Vec<WheelState>,
	pub dribber_speed: f64,
	pub kicker_position: Vec<f64>,
	pub kicker_velocity: Vec<f64>,
}

#[derive(Default)]
pub struct BallState {
	pub ball_position: Vec<f64>,
	pub ball_spin: Vec<f64>,
	pub ball_velocity: Vec<f64>,
}

#[derive(Default)]
pub struct ScoreState {
	pub friendly_score: u16, // total friendly score
	pub friendly_out_of_bounds_score: u16, // total friendly score from enemy bot OOB
	pub friendly_goal_score: u16, // total friendly score from goal
	pub enemy_score: u16, // total enemy score
	pub enemy_out_of_bounds_score: u16, // total enemy score from enemy bot OOB
	pub enemy_goal_score: u16, // total friendly score from goal
}

#[derive(Default)]
pub struct WheelState {
	pub big_wheel_velocity: f64,
	pub big_wheel_orientation: f64,
	pub small_wheel_velocity: Vec<f64>,
}