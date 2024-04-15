use crate::data;
use config::Config;
use crate::bot_geom_load;

pub fn simulate (
	game_state: data::GameState,
	friendly_bot_1_action: data::Action, friendly_bot_2_action: data::Action,
	enemy_bot_1_action: data::Action, enemy_bot_2_action: data::Action,
	friendly_bot_1_geom: bot_geom_load::BotGeomData, friendly_bot_2_geom: bot_geom_load::BotGeomData,
	enemy_bot_1_geom: bot_geom_load::BotGeomData, enemy_bot_2_geom: bot_geom_load::BotGeomData,
) -> data::GameState {

	let next_game_state: data::GameState = data::GameState::default();

	return next_game_state
}