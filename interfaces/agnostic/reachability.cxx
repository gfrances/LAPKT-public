#include <reachability.hxx>
#include <action.hxx>

namespace aptk {

namespace agnostic {

Reachability_Test::Reachability_Test( const STRIPS_Problem& p )
	: m_problem( p )
{
	m_reachable_atoms.resize( m_problem.fluents().size() );
	m_reach_next.resize( m_problem.fluents().size() );
	m_action_mask.resize( m_problem.actions().size() );
}

Reachability_Test::~Reachability_Test()
{
}

void	Reachability_Test::initialize( const Fluent_Vec& s)
{
	for ( unsigned i = 0; i < m_reachable_atoms.size(); i++ )
		m_reachable_atoms[i] = false;

	m_action_mask.reset();
	
	for ( unsigned i = 0; i < s.size(); i++ ) {

		#ifdef DEBUG
		std::cout << "Fluent " << m_problem.fluents()[s[i]]->signature() << " initially true" << std::endl;
		#endif
		m_reachable_atoms[ s[i] ] = true;
	}
}

bool	Reachability_Test::apply_actions() {

	bool fixed_point = true;
	m_reach_next = m_reachable_atoms;

	for ( unsigned i = 0; i < m_problem.actions().size(); i++ )
	{
		if ( m_action_mask.isset(i) ) continue;

		const Action* a = m_problem.actions()[i];
		
		// Check if applicable
		const Fluent_Vec&	pi = a->prec_vec();
		bool		applicable = true;
		for ( unsigned j = 0; j < pi.size(); j++ )
			if ( !m_reachable_atoms[pi[j]] )
			{
				applicable = false;
				break;
			}
		
		if ( !applicable ) continue;
		
		#ifdef DEBUG
		std::cout << "Applying " << a->signature() << std::endl;
		#endif
		
		// Apply effects
		const Fluent_Vec& ai = a->add_vec();
		for ( unsigned j = 0; j < ai.size(); j++ ) {
			if ( !m_reachable_atoms[ai[j]] ) {
				m_reach_next[ai[j]] = true;
				fixed_point = false;
			}
		}

		bool all_ce_applied = true;

		for( unsigned j = 0; j < a->ceff_vec().size(); j++ ) {

			const Fluent_Vec&	pi = a->ceff_vec()[j]->prec_vec();
			bool			applicable = true;

			for ( unsigned k = 0; k < pi.size(); k++ )
				if ( !m_reachable_atoms[pi[k]] ) {
					applicable = false;
					break;
				}
		
			if ( !applicable ) {
				all_ce_applied = false;
				continue;
			}

			const Fluent_Vec&	ai = a->ceff_vec()[j]->add_vec();

			for ( unsigned k = 0; k < ai.size(); k++ ) {

				if ( !m_reachable_atoms[ai[k]] ) {

					m_reach_next[ai[k]] = true;
					fixed_point = false;
				}
			}
	
		}
		if ( all_ce_applied ) m_action_mask.set(i);
	}
	m_reachable_atoms = m_reach_next;
	return fixed_point;
}

void	Reachability_Test::print_reachable_atoms() {

	for (unsigned k = 0; k < m_reachable_atoms.size(); k++ )
		if ( m_reachable_atoms[k] )
			std::cout << m_problem.fluents()[k]->signature() << std::endl;	
}

bool	Reachability_Test::is_reachable( const Fluent_Vec& s, const Fluent_Vec& g ) {

	initialize(s);

	#ifdef DEBUG
	std::cout << "Reachable atoms:" << std::endl;
	print_reachable_atoms();
	#endif	
	
	while ( !apply_actions() )
	{
		#ifdef DEBUG
		std::cout << "Reachable atoms:" << std::endl;
		print_reachable_atoms();
		#endif	

		if ( check( g ) )	
			return true;
	}
	return check(g);
}

bool	Reachability_Test::is_reachable( const Fluent_Vec& s, const Fluent_Vec& g, const unsigned op )
{
	initialize(s);

	#ifdef DEBUG
	std::cout << "Disabling operator " << m_problem.actions()[op]->signature() << std::endl;
	#endif
	m_action_mask.set(op);	
	#ifdef DEBUG
	std::cout << "Reachable atoms:" << std::endl;
	print_reachable_atoms();
	#endif	

	while ( !apply_actions() ) {
		#ifdef DEBUG
		std::cout << "Reachable atoms:" << std::endl;
		print_reachable_atoms();
		#endif	

		if ( check( g ) )	
			return true;
	}
	return check(g);
}

bool	Reachability_Test::is_reachable( const Fluent_Vec& s, const Fluent_Vec& g, const Bit_Set& excluded )
{
	initialize(s);

	m_action_mask.add(excluded);	
	#ifdef DEBUG
	std::cout << "Reachable atoms:" << std::endl;
	print_reachable_atoms();
	#endif	

	while ( !apply_actions() ) {
		#ifdef DEBUG
		std::cout << "Reachable atoms:" << std::endl;
		print_reachable_atoms();
		#endif	

		if ( check( g ) )	
			return true;
	}
	return check(g);
}


bool	Reachability_Test::check( const Fluent_Vec& g )  {

	for ( unsigned i = 0; i < g.size(); i++ )
		if ( !m_reachable_atoms[ g[i] ] )
			return false;
	return true;
}


}

}
