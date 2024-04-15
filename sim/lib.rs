mod data;
mod take_view;
mod simulate;
mod bot_geom_load;

extern crate config;

pub fn next_step (
	game_state: data::GameState,
	friendly_bot_1_action: data::Action, friendly_bot_2_action: data::Action,
	enemy_bot_1_action: data::Action, enemy_bot_2_action: data::Action,
) -> (data::GameState, data::ViewState,
	data::ViewState, data::ViewState, data::ViewState, ) {
	
	let next_game_state: data::GameState = simulate::simulate(game_state,
		friendly_bot_1_action, friendly_bot_2_action,
		enemy_bot_1_action, enemy_bot_2_action
	);
	
	let (friendly_bot_1_view, friendly_bot_2_view,
		enemy_bot_1_view, enemy_bot_2_view) = take_view::take_view(next_game_state);
	
	(next_game_state,
		friendly_bot_1_view, friendly_bot_2_view,
		enemy_bot_1_view, enemy_bot_2_view)
}