#ifndef LOCIC_CODEGEN_CONSTANTGENERATOR_HPP
#define LOCIC_CODEGEN_CONSTANTGENERATOR_HPP

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
	
		class ConstantGenerator {
			public:
				inline ConstantGenerator(const Module& module)
					: module_(module) { }
				
				inline llvm::UndefValue* getVoidUndef() const {
					return llvm::UndefValue::get(
						llvm::Type::getVoidTy(module_.getLLVMContext()));
				}
				
				inline llvm::UndefValue* getUndef(llvm::Type* type) const {
					return llvm::UndefValue::get(type);
				}
				
				inline llvm::Constant* getNull(llvm::Type* type) const {
					return llvm::Constant::getNullValue(type);
				}
				
				inline llvm::ConstantPointerNull* getNullPointer(llvm::PointerType* pointerType) const {
					return llvm::ConstantPointerNull::get(pointerType);
				}
				
				inline llvm::ConstantInt* getInt(size_t sizeInBits, long long intValue) const {
					assert(sizeInBits >= 1);
					return llvm::ConstantInt::get(module_.getLLVMContext(),
						llvm::APInt(sizeInBits, intValue));
				}
				
				inline llvm::ConstantInt* getI1(bool value) const {
					return getInt(1, value ? 1 : 0);
				}
				
				inline llvm::ConstantInt* getI8(uint8_t value) const {
					return getInt(8, value);
				}
				
				inline llvm::ConstantInt* getI32(uint32_t value) const {
					return getInt(32, value);
				}
				
				inline llvm::ConstantInt* getI64(uint64_t value) const {
					return getInt(64, value);
				}
				
				inline llvm::ConstantInt* getSizeTValue(unsigned long long sizeValue) const {
					const size_t sizeTypeWidth = module_.getTargetInfo().getPrimitiveSize("size_t");
					return getInt(sizeTypeWidth, sizeValue);
				}
				
				inline llvm::ConstantInt* getPrimitiveInt(const std::string& primitiveName, long long intValue) const {
					const size_t primitiveWidth = module_.getTargetInfo().getPrimitiveSize(primitiveName);
					return getInt(primitiveWidth, intValue);
				}
				
				inline llvm::Constant* getPrimitiveFloat(const std::string& primitiveName, long double floatValue) const {
					if (primitiveName == "float_t") {
						return getFloat(floatValue);
					} else if (primitiveName == "double_t") {
						return getDouble(floatValue);
					} else if (primitiveName == "longdouble_t") {
						return getLongDouble(floatValue);
					} else {
						assert(false && "TODO");
						return NULL;
					}
				}
				
				inline llvm::Constant* getFloat(float value) const {
					return llvm::ConstantFP::get(module_.getLLVMContext(), llvm::APFloat(value));
				}
				
				inline llvm::Constant* getDouble(double value) const {
					return llvm::ConstantFP::get(module_.getLLVMContext(), llvm::APFloat(value));
				}
				
				inline llvm::Constant* getLongDouble(long double value) const {
					return llvm::ConstantFP::get(module_.abi().longDoubleType(), value);
				}
				
				inline llvm::Constant* getArray(llvm::ArrayType* arrayType, const std::vector<llvm::Constant*>& values) const {
					return llvm::ConstantArray::get(arrayType, values);
				}
				
				inline llvm::Constant* getStruct(llvm::StructType* structType, const std::vector<llvm::Constant*>& values) const {
					return llvm::ConstantStruct::get(structType, values);
				}
				
				inline llvm::Constant* getString(const std::string& value, bool withNullTerminator = true) const {
					return llvm::ConstantDataArray::getString(module_.getLLVMContext(),
						value, withNullTerminator);
				}
				
				inline llvm::Constant* getPointerCast(llvm::Constant* value, llvm::Type* targetType) const {
					return llvm::ConstantExpr::getPointerCast(value, targetType);
				}
				
				inline llvm::Constant* getAlignOf(llvm::Type* type) const {
					return llvm::ConstantExpr::getAlignOf(type);
				}
				
				inline llvm::Constant* getSizeOf(llvm::Type* type) const {
					return llvm::ConstantExpr::getSizeOf(type);
				}
				
				inline llvm::Constant* getGetElementPtr(llvm::Constant* operand, const std::vector<llvm::Constant*>& args) const {
					return llvm::ConstantExpr::getGetElementPtr(operand, args);
				}
				
				inline llvm::Constant* getMin(llvm::Constant* first, llvm::Constant* second) const {
					llvm::Constant* compareResult = llvm::ConstantExpr::getICmp(llvm::CmpInst::ICMP_ULT, first, second);
					return llvm::ConstantExpr::getSelect(compareResult, first, second);
				}
				
				inline llvm::Constant* getMax(llvm::Constant* first, llvm::Constant* second) const {
					llvm::Constant* compareResult = llvm::ConstantExpr::getICmp(llvm::CmpInst::ICMP_UGT, first, second);
					return llvm::ConstantExpr::getSelect(compareResult, first, second);
				}
				
			private:
				const Module& module_;
				
		};
		
	}
	
}

#endif
