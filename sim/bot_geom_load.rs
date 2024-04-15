use std::fs::File;
use serde_json;

pub fn load_geom (config_path: &str) -> BotGeomData {
    let file: File = File::open(config_path).unwrap();
    let cfg_json: serde_json::Value = serde_json::from_reader(file).unwrap();

    // precomp hitboxes

}

pub struct BotGeomData {
    // all speed, torque etc is in standard units (rad/s, etc.)

    bot_radius: f64,
    bot_mass: f64,
    bot_front_insink_depth: f64,
	bot_front_insink_inner_width: f64,
	bot_front_insink_outer_width: f64,
    bot_hitbox: Vec<f64>,

    // no rolling friction / bearing friction
    wheel_fric_s: f64,
    wheel_fric_k: f64,
    wheel_max_speed: f64,
    wheel_max_torque: f64,
    small_wheel_fric_s: f64,
    small_wheel_fric_k: f64,
    n_wheels: u8,
    
    // assuming that the dribbler double cone is straight half edge
    // centering tendency should be set to 0 unless otherwise
    have_dribbler: bool,
    dribbler_edge_radius: f64,
    dribbler_middle_radius: f64,
    dribbler_hitbox: Vec<f64>,
    dribbler_centering_tendency: f64,
    dribbler_width: f64,
    dribbler_max_speed: f64,
    dribbler_down_pos: Vec<f64>,
    dribbler_pivot_pos: Vec<f64>,
    dribbler_max_pivot: f64,

    have_kicker: bool,
    kicker_max_impulse: f64,
    kicker_min_impulse: f64,
    kicker_elasticity: f64,
    kicker_uncertainty: f64,

    ball_mass: f64,
    ball_radius: f64,
    ball_hitbox: f64,
    ball_elasticity: f64,
    ball_fric_s: f64,
    ball_fric_k: f64,
}