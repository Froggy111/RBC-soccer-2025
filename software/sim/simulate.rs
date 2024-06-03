use crate::data;
use crate::bot_geom_load;
use rapier3d::prelude::*;
use std::collections::HashMap;

pub struct Simulator<'a> {
	physics_pipeline: PhysicsPipeline,

	gravity: Vector<Real>,
	integration_parameters: IntegrationParameters,

	island_manager: IslandManager,
	broad_phase: BroadPhase,
	narrow_phase: NarrowPhase,
	rigid_body_set: RigidBodySet,
	collider_set: ColliderSet,
	impulse_joint_set: ImpulseJointSet,
	multibody_joint_set: MultibodyJointSet,
	ccd_solver: CCDSolver,
	query_pipeline: QueryPipeline,
	physics_hooks: &'a dyn PhysicsHooks,
	event_handler: &'a dyn EventHandler,
	
	colliders: HashMap<&'a str, ColliderHandle>,
	rigid_bodies: HashMap<&'a str, RigidBodyHandle>,
}

impl Simulator<'_> {
	
	pub fn new () -> Self {
		Self {
			physics_pipeline: PhysicsPipeline::new(),

			gravity: vector![0.0, -9.81, 0.0],
			integration_parameters: IntegrationParameters::default(),

			island_manager: IslandManager::new(),
			broad_phase: BroadPhase::new(),
			narrow_phase: NarrowPhase::new(),
			rigid_body_set: RigidBodySet::new(),
			collider_set: ColliderSet::new(),
			impulse_joint_set: ImpulseJointSet::new(),
			multibody_joint_set: MultibodyJointSet::new(),
			ccd_solver: CCDSolver::new(),
			query_pipeline: QueryPipeline::new(),
			physics_hooks: &(),
			event_handler: &(),

			colliders: HashMap::<&str, ColliderHandle>::new(),
			rigid_bodies: HashMap::<&str, RigidBodyHandle>::new(),
		}
	}

	pub fn step (&self) -> () {
		self.physics_pipeline.step (
			&self.gravity,
			&self.integration_parameters,
			&mut self.island_manager,
			&mut self.broad_phase, 
			&mut self.narrow_phase,
			&mut self.rigid_body_set,
			&mut self.collider_set,
			&mut self.impulse_joint_set,
			&mut self.multibody_joint_set,
			&mut self.ccd_solver,
			Some(&mut self.query_pipeline), 
			self.physics_hooks,
			self.event_handler)		
	}

	pub fn insert_rigid_body (
		&self, rigid_body: RigidBody, rigid_body_name: &str,) -> () {
		let rigid_body_handle = self.rigid_body_set.insert(rigid_body);
		self.rigid_bodies.insert(rigid_body_name, rigid_body_handle);
	}

	pub fn insert_collider (
		&self, collider: Collider, collider_name: &str,) -> () {
		let collider_handle = self.collider_set.insert(collider);
		self.colliders.insert(collider_name, collider_handle);
	}

	pub fn insert_collider_with_parent (
		&self, collider: Collider, collider_name: &str, parent_name: &str
	) -> () {
		let collider_handle = self.collider_set.insert_with_parent(collider, self.rigid_bodies[&parent_name], &mut self.rigid_body_set);
		self.colliders.insert(collider_name, collider_handle);
	}

	pub fn 
}


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