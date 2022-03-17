/*Handle all hit reactions: triggerStagger, knock back, guard break etc*/
class reactionHandler {
private:
	static enum poiseReactionType
	{
		kSmall = 0,
		kMedium,
		kLarge,
		kLargest //knockBack
	};

	/*Stagger the reactor with magnitude and correct stagger direction.
	@param causer: the cause of the stagger.
	@param reactor: actor who staggers.
	@param magnitude: magnitude of the triggerStagger.*/
	static void triggerStagger(RE::Actor* causer, RE::Actor* reactor, float magnitude);

	/*Inflict a "poise" reaction onto reactor.
	@param causer: the cause of the reaction.
	@param reactor: actor who react.
	@param reactionType: type of the poise reaction.*/
	static void triggerPoiseReaction(RE::Actor* causer, RE::Actor* reactor, poiseReactionType reactionType);
public:

	/*Initialize a small reaction.*/
	static void triggerReactionSmall(RE::Actor* causer, RE::Actor* reactor);

	/*Initialize a medium reaction.*/
	static void triggerReactionMedium(RE::Actor* causer, RE::Actor* reactor);

	/*Initialize a large reaction. */
	static void triggerReactionLarge(RE::Actor* causer, RE::Actor* reactor);

	/*Initialize a knockback.*/
	static void triggerKnockBack(RE::Actor* causer, RE::Actor* reactor);


	
};