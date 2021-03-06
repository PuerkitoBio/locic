#ifndef LOCIC_CODEGEN_PENDINGRESULT_HPP
#define LOCIC_CODEGEN_PENDINGRESULT_HPP

#include <locic/Support/Array.hpp>

namespace locic {
	
	namespace SEM {
		
		class Type;
		
	}
	
	namespace CodeGen {
		
		class Function;
		
		class PendingResultBase {
		public:
			/**
			 * \brief Generate value.
			 * 
			 * This requests that a value is generated, which would
			 * ideally be constructed in the hint result value given
			 * (if it isn't null).
			 * 
			 * \param function The function generator.
			 * \param hintResultValue A hint of where the result
			 *                        should be constructed to
			 *                        minimise moves, if not null.
			 * \return The generated LLVM value.
			 */
			virtual llvm::Value* generateValue(Function& function, llvm::Value* hintResultValue) const = 0;
			
			/**
			 * \brief Generate loaded value.
			 * 
			 * This requests that a value is generated and loaded;
			 * it exists alongside 'generate' since it is possible
			 * in some cases to avoid store-load pairs by knowing
			 * in advance that they need to be loaded.
			 * 
			 * \param function The function generator.
			 * \return The generated LLVM value.
			 */
			virtual llvm::Value* generateLoadedValue(Function& function) const = 0;
			
		protected:
			// Prevent destruction via this class.
			~PendingResultBase() { }
			
		};
		
		class RefPendingResult: public PendingResultBase {
		public:
			RefPendingResult(llvm::Value* value, const SEM::Type* refTargetType);
			
			llvm::Value* generateValue(Function& function, llvm::Value* hintResultValue) const;
			
			llvm::Value* generateLoadedValue(Function& function) const;
			
		private:
			llvm::Value* value_;
			const SEM::Type* refTargetType_;
			
		};
		
		class ValuePendingResult: public PendingResultBase {
		public:
			ValuePendingResult(llvm::Value* value, const SEM::Type* type);
			
			llvm::Value* generateValue(Function& function, llvm::Value* hintResultValue) const;
			
			llvm::Value* generateLoadedValue(Function& function) const;
			
		private:
			llvm::Value* value_;
			const SEM::Type* type_;
			
		};
		
		/**
		 * \brief Pending result.
		 * 
		 * This class provides a way to delay the generation
		 * of instructions until a value is required; this
		 * provides a mechanism to *avoid* generating code
		 * that isn't required. For example, consider:
		 * 
		 * 1.add(2);
		 * 
		 * This code requires binding both of the integer
		 * values to references, even though primitive method
		 * generation then simply loads those values. By using
		 * a pending result, the bind operation is delayed
		 * and then when the value is requested to be loaded
		 * the bind is omitted altogether.
		 */
		class PendingResult {
		public:
			PendingResult(const PendingResultBase& base);
			
			llvm::Value* resolve(Function& function, llvm::Value* hintResultValue = nullptr);
			
			llvm::Value* resolveWithoutBind(Function& function);
			
		private:
			const PendingResultBase* base_;
			llvm::Value* cacheLastHintResultValue_;
			llvm::Value* cacheLastResolvedValue_;
			llvm::Value* cacheLastResolvedWithoutBindValue_;
			
		};
		
		using PendingResultArray = Array<PendingResult, 10>;
		
	}
	
}

#endif
