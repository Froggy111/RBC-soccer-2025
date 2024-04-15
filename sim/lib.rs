mod data;
mod take_view;
mod simulate;
mod bot_geom_load;

/*

Idk how this will work but

some ideas (model)
do all environmental info processing first - model will only "decide strategy"
- throw transformer at it and it will probably work? - do this first to see emergent behaviors? do MoE later?
	- attn encodes multiple frames of views? look into architectures
	- probably quite low dimensional? refer to data.rs
- throw transformer with it but action calls? - if anything works this is probably gonna be what is done
- markov chain (?) the problem could be too complicated tho - last resort or just give up at this point

sim
- a "true" game state - fully describes the current state of the game
- slightly noisy views to simulate irl noisy inputs
- to be accurate will need to be high frame per "rl"-second - internally iterate for more frames

*/

pub fn next_step (
	game_state: data::GameState,
	friendly_bot_1_action: data::Action, friendly_bot_2_action: data::Action,
	enemy_bot_1_action: data::Action, enemy_bot_2_action: data::Action,
	friendly_bot_1_geom: bot_geom_load::BotGeomData, friendly_bot_2_geom: bot_geom_load::BotGeomData,
	enemy_bot_1_geom: bot_geom_load::BotGeomData, enemy_bot_2_geom: bot_geom_load::BotGeomData,
) -> (data::GameState, data::ViewState,
	data::ViewState, data::ViewState, data::ViewState, ) {
	
	let next_game_state: data::GameState = simulate::simulate(game_state,
		friendly_bot_1_action, friendly_bot_2_action,
		enemy_bot_1_action, enemy_bot_2_action,
		friendly_bot_1_geom, friendly_bot_2_geom,
		enemy_bot_1_geom, enemy_bot_2_geom
	);
	
	let (friendly_bot_1_view, friendly_bot_2_view,
		enemy_bot_1_view, enemy_bot_2_view) = take_view::take_view(next_game_state);
	
	(next_game_state,
		friendly_bot_1_view, friendly_bot_2_view,
		enemy_bot_1_view, enemy_bot_2_view)
}