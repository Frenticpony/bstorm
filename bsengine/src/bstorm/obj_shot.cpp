﻿#include <bstorm/obj_shot.hpp>

#include <bstorm/interpolation.hpp>
#include <bstorm/dnh_const.hpp>
#include <bstorm/math_util.hpp>
#include <bstorm/dx_util.hpp>
#include <bstorm/renderer.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/dnh_value.hpp>
#include <bstorm/script.hpp>
#include <bstorm/obj_player.hpp>
#include <bstorm/obj_item.hpp>
#include <bstorm/intersection.hpp>
#include <bstorm/shot_data.hpp>
#include <bstorm/rand_generator.hpp>
#include <bstorm/shot_counter.hpp>
#include <bstorm/package.hpp>

#include <algorithm>
#include <d3dx9.h>

namespace bstorm
{
ObjShot::ObjShot(bool isPlayerShot, bool isECLShot, const std::shared_ptr<CollisionDetector>& colDetector, const std::shared_ptr<Package>& package) :
    ObjRender(package),
    ObjMove(this),
    ObjCol(colDetector),
    isPlayerShot_(isPlayerShot),
    isECLShot_(isECLShot),
	initX_(0),
	initY_(0),
	initSpeed_(0),
	initAngle_(0),
	initDelay_(0),
	initFadeDelete_(0),
	fadeX_(0),
	fadeY_(0),
	fadeRandA_(0),
	fadeRandB_(0),
	fadeRandC_(0),
	fadeRandD_(0),
	useSelfDelayRect_(false),
	useSelfFadeRect_(false),
	FadeEx{ 1.0f, 1.65f, 0.5f, 1.0f },
    isRegistered_(false),
    intersectionEnable_(true),
    autoDeleteEnable_(true),
	autoDeleteTimer_(-1), //FP AUTO TIMER
	spellResistTimer_(-1), //FP SPELL RESIST DELAY
    spellResistEnable_(false),
    spellFactorEnable_(false),
    itemChangeEnable_(true),
    angularVelocity_(0),
    damage_(1),
    penetration_(5),
    eraseShotEnable_(false),
    delayTimer_(0),
	delayCounter_(0),
	fadeDeleteCounter_(0),
    deleteFrameTimer_(0),
    isFrameDeleteStarted_(false),
    sourceBlendType_(BLEND_NONE),
    isGrazeInvalid_(false),
    isTempIntersectionMode_(false),
    addedShotFrameCnt_(0),
    isFadeDeleteStarted_(false),
    fadeDeleteFrame_(32),
    animationFrameCnt_(0),
    animationIdx_(0)
{
    SetType(OBJ_SHOT);
    SetBlendType(BLEND_NONE);
	SetFilterType(FILTER_LINEAR); //FP FILTER
    if (isPlayerShot_)
    {
        package->SuccPlayerShotCount();
    } else
    {
        package->SuccEnemyShotCount();
    }
}

ObjShot::~ObjShot()
{
}

void ObjShot::Update()
{
	if (IsRegistered())
	{
		if(GetPenetration() <= 0) { Die(); return; }
		if(!IsDelay())
		{
			if (!IsFadeDeleteStarted())
			{
				if(!isECLShot_)
				{
					Move();
				}
				else
				{
					ECLMove();
				}
			}
			else
			{
				SetSpeed(GetSpeed() * 0.925f);
				MoveFade();
			}
			CheckAutoDelete(GetX(), GetY());

			if (shotData_)
			{
				SetAngleZ(GetAngleZ() + GetAngularVelocity());
				UpdateAnimationPosition();
			}
		}
		else
		{
			float d_maxrad = (initSpeed_ * 0.5f) * initDelay_;

			/* Delay Position Interpolation */
			float d_rad = easeLinear(0.0f, d_maxrad, delayCounter_, initDelay_);

			SetMovePosition(initX_ + (d_rad * cos(D3DXToRadian(initAngle_))), initY_ + (d_rad * sin(D3DXToRadian(initAngle_))));
		}
		TickAddedShotFrameCount();
		TickDelayTimer();
		TickDeleteFrameTimer();
		TickAutoDeleteTimer(); //FP AUTO TIMER
		TickSpellResistTimer(); //FP SPELL RESIST DELAY
		TickFadeDeleteTimer();
	}
	UpdateTempIntersection();
}

void ObjShot::OnDead() noexcept
{
    if (auto package = GetPackage().lock())
    {
        for (auto& addedShot : addedShots_)
        {
            package->DeleteObject(addedShot.objId);
        }
        if (IsPlayerShot())
        {
            package->PredPlayerShotCount();
        } else
        {
            package->PredEnemyShotCount();
        }
    }
}

void ObjShot::Render(const std::shared_ptr<Renderer>& renderer)
{
    if (IsRegistered())
    {
        if (shotData_)
        {
			int shotBlend = BLEND_NONE;
			int shotFilter = FILTER_LINEAR; //FP FILTER
			D3DCOLOR color;
			float delayScale = 1.0f;
			float fadeScale = 1.0f;
			float fadeAlpha = 1.0f;

			if(!IsDelay() || IsFadeDeleteStarted())
			{
				if (GetBlendType() == BLEND_NONE)
				{
					shotBlend = shotData_->render;
				}
				else
				{
					shotBlend = GetBlendType();
				}

				if (GetFilterType() == FILTER_LINEAR)
				{
					shotFilter = shotData_->filter;
				}
				else
				{
					shotFilter = GetFilterType();
				}

				if (IsFadeDeleteStarted())
				{
					fadeScale = easeLinear((float)shotData_->fadeData.vB, (float)shotData_->fadeData.vC, std::min(fadeDeleteCounter_, 12), 12);
					fadeAlpha = easeLinear((float)shotData_->fadeData.vD, (float)shotData_->fadeData.vE, std::min(fadeDeleteCounter_, 12), 12);
					shotBlend = shotData_->fadeRender;
					shotFilter = FILTER_LINEAR;

					if (shotData_->useExFade)
					{
						FadeExDraw(renderer);
					}
				}
				
				// NOTE: Interpret ADD_RGB as ADD_ARGB
				if (GetBlendType() == BLEND_NONE && shotBlend == BLEND_ADD_RGB)
				{
					shotBlend = BLEND_ADD_ARGB;
				}

				color = GetColor().ToD3DCOLOR((int)(fadeAlpha * std::min(shotData_->alpha, GetAlpha())));
				D3DXMATRIX world = CreateScaleRotTransMatrix(GetX(), GetY(), 0.0f, GetAngleX(), GetAngleY(), GetAngleZ() + ((IsFadeDeleteStarted() && !shotData_->fixedAngle) ? fadeRandB_ : shotData_->fixedAngle ? 0.0f : GetAngle() + 90.0f), IsFadeDeleteStarted() ? fadeScale : GetScaleX(), IsFadeDeleteStarted() ? fadeScale : GetScaleY(), 1.0f);
				auto vertices = GetRectVertices(color, shotData_->texture->GetWidth(), shotData_->texture->GetHeight(), IsDelay() ? shotData_->delayRect : (animationIdx_ >= 0 && animationIdx_ < shotData_->animationData.size()) ? shotData_->animationData[animationIdx_].rect : (IsFadeDeleteStarted() && !shotData_->useSelfFadeRect) ? shotData_->fadeRect : shotData_->rect );
				renderer->RenderPrim2D(D3DPT_TRIANGLESTRIP, 4, vertices.data(), shotData_->texture->GetTexture(), shotBlend, shotFilter, world, GetAppliedShader(), IsPermitCamera(), true);
			}
			else
			{
				if (GetSourceBlendType() == BLEND_NONE)
				{
					shotBlend = shotData_->delayRender;
				}
				else
				{
					shotBlend = GetSourceBlendType();
				}

				shotFilter = FILTER_LINEAR;

				/* Delay Scale Interpolation */
				//float delayScale = easeOutCubic(2.75f, 0.5f, delayCounter_, initDelay_);
				float delayScale = easeOutCubic((float)shotData_->delayData.vA, (float)shotData_->delayData.vB, delayCounter_, initDelay_);
				
				/* Delay Alpha Interpolation */
				float delayAlpha = easeLinear(75.0f, 255.0f, delayCounter_, initDelay_);

				// In the case of a curve laser, the delay scale is slightly larger
				if (GetType() == OBJ_CURVE_LASER)
				{
					delayScale *= 1.8f;
				}

				color = GetColor().ToD3DCOLOR((int)(delayAlpha));

				D3DXMATRIX world = CreateScaleRotTransMatrix(GetX(), GetY(), 0.0f,
					GetAngleX(), GetAngleY(), (shotData_->useSelfDelayRect && !shotData_->fixedAngle) ? GetAngle() + 90.0f : GetAngle(),
					IsDelay() ? delayScale : GetScaleX(), IsDelay() ? delayScale : GetScaleY(), 1.0f);

				// NOTE: Interpret ADD_RGB as ADD_ARGB
				if (GetBlendType() == BLEND_NONE && shotBlend == BLEND_ADD_RGB)
				{
					shotBlend = BLEND_ADD_ARGB;
				}

				auto vertices = GetRectVertices(color, shotData_->texture->GetWidth(), shotData_->texture->GetHeight(), shotData_->useSelfDelayRect ? shotData_->rect : shotData_->delayRect);

				renderer->RenderPrim2D(D3DPT_TRIANGLESTRIP, 4, vertices.data(), shotData_->texture->GetTexture(), shotBlend, shotFilter, world, GetAppliedShader(), IsPermitCamera(), true);
			}
        }
        RenderIntersection(renderer);
    }
}

void ObjShot::FadeExDraw(const std::shared_ptr<Renderer>& renderer)
{
	// ----- Fade Delete Extra Sprite -----
	
	float fadeScaleSub;
	float fadeAlphaSub = 160.0f;
	float fadeXOff = 0.0f;
	float fadeYOff = 0.0f;

	if (fadeDeleteCounter_ <= 8)
	{
		fadeScaleSub = easeLinear(FadeEx.effFadeExStart, FadeEx.effFadeExMiddle, fadeDeleteCounter_, 8);
	}
	else
	{
		fadeScaleSub = easeInCubic(FadeEx.effFadeExMiddle, FadeEx.effFadeExEnd, fadeDeleteCounter_ - 8, 16);
		fadeAlphaSub = easeInCubic(16.0f, 0.0f, fadeDeleteCounter_ - 8, 16);
	}
	fadeXOff = easeInCubic(0.0f, fadeRandC_ - 10, fadeDeleteCounter_, 38);
	fadeYOff = easeOutCubic(0.0f, 42.0f - (fadeRandD_), fadeDeleteCounter_, 38);

	D3DCOLOR colorFade = GetColor().ToD3DCOLOR((int)(fadeAlphaSub));
	D3DXMATRIX worldF = CreateScaleRotTransMatrix(fadeX_ + fadeXOff, fadeY_ - fadeYOff, 0.0f, GetAngleX(), GetAngleY(), fadeRandA_, fadeScaleSub, fadeScaleSub, 1.0f);
	auto verticesF = GetRectVertices(colorFade, shotData_->texture->GetWidth(), shotData_->texture->GetHeight(), shotData_->fadeRect);
	renderer->RenderPrim2D(D3DPT_TRIANGLESTRIP, 4, verticesF.data(), shotData_->texture->GetTexture(), BLEND_ADD_ARGB, FILTER_LINEAR, worldF, GetAppliedShader(), IsPermitCamera(), true);
}

double ObjShot::GetDamage() const
{
    return damage_;
}

void ObjShot::SetDamage(double damage) { this->damage_ = damage; }

int ObjShot::GetPenetration() const
{
    return penetration_;
}

void ObjShot::SetPenetration(int penetration) { this->penetration_ = penetration; }

int ObjShot::GetDelay() const
{
    return delayTimer_;
}

void ObjShot::SetDelay(int delay) { delayTimer_ = std::max(delay, 0); delayCounter_ = 0; }

bool ObjShot::IsDelay() const
{
    return delayTimer_ > 0;
}

int ObjShot::GetSourceBlendType() const
{
    return sourceBlendType_;
}

void ObjShot::SetSourceBlendType(int blendType)
{
    sourceBlendType_ = blendType;
}

float ObjShot::GetInitX() const
{
	return initX_;
}

float ObjShot::GetInitY() const
{
	return initY_;
}

float ObjShot::GetInitSpeed() const
{
	return initSpeed_;
}

float ObjShot::GetInitAngle() const
{
	return initAngle_;
}

int ObjShot::GetInitDelay() const
{
	return initDelay_;
}

float ObjShot::GetAngularVelocity() const
{
    return angularVelocity_;
}

void ObjShot::SetAngularVelocity(float angularVelocity)
{
    this->angularVelocity_ = angularVelocity;
}

bool ObjShot::IsSpellResistEnabled() const { return spellResistEnable_; }

void ObjShot::SetSpellResistEnable(bool enable) { spellResistEnable_ = enable; }

bool ObjShot::IsSpellFactorEnabled() const { return spellFactorEnable_; }

void ObjShot::SetSpellFactor(bool enable) { spellFactorEnable_ = enable; }

bool ObjShot::IsEraseShotEnabled() const { return eraseShotEnable_; }

void ObjShot::SetEraseShotEnable(bool enable)
{
    eraseShotEnable_ = enable;
    if (IsPlayerShot())
    {
        // 既に作られている判定に弾消し属性を付与
        for (auto& isect : ObjCol::GetIntersections())
        {
            if (auto shotIsect = std::dynamic_pointer_cast<ShotIntersection>(isect))
            {
                shotIsect->SetEraseShotEnable(enable);
            }
        }

        for (auto& isect : ObjCol::GetTempIntersections())
        {
            if (auto shotIsect = std::dynamic_pointer_cast<ShotIntersection>(isect))
            {
                shotIsect->SetEraseShotEnable(enable);
            }
        }
    }
}

bool ObjShot::IsItemChangeEnabled() const
{
    return itemChangeEnable_;
}

void ObjShot::SetItemChangeEnable(bool enable) { itemChangeEnable_ = enable; }

bool ObjShot::IsAutoDeleteEnabled() const
{
    return autoDeleteEnable_;
}

void ObjShot::SetAutoDeleteEnable(bool enable)
{
    autoDeleteEnable_ = enable;
}

void ObjShot::AddIntersection(const std::shared_ptr<ShotIntersection>& isect)
{
    if (!isTempIntersectionMode_)
    {
        ObjCol::AddIntersection(isect);
    }
}

void ObjShot::AddTempIntersection(const std::shared_ptr<ShotIntersection>& isect)
{
    if (!isTempIntersectionMode_)
    {
        isTempIntersectionMode_ = true;
        ClearIntersection();
    }
    ObjCol::AddTempIntersection(isect);
}

void ObjShot::AddIntersectionCircleA1(float r)
{
    AddIntersectionCircleA2(GetX(), GetY(), r);
}

void ObjShot::AddIntersectionCircleA2(float x, float y, float r)
{
    AddIntersection(std::make_shared<ShotIntersection>(x, y, r, shared_from_this(), false));
}

void ObjShot::AddIntersectionLine(float x1, float y1, float x2, float y2, float width)
{
    AddIntersection(std::make_shared<ShotIntersection>(x1, y1, x2, y2, width, shared_from_this(), false));
}

void ObjShot::AddTempIntersectionCircleA1(float r)
{
    AddTempIntersectionCircleA2(GetX(), GetY(), r);
}

void ObjShot::AddTempIntersectionCircleA2(float x, float y, float r)
{
    AddTempIntersection(std::make_shared<ShotIntersection>(x, y, r, shared_from_this(), true));
}

void ObjShot::AddTempIntersectionLine(float x1, float y1, float x2, float y2, float width)
{
    AddTempIntersection(std::make_shared<ShotIntersection>(x1, y1, x2, y2, width, shared_from_this(), true));
}

bool ObjShot::IsIntersectionEnabled() const
{
    return intersectionEnable_;
}

void ObjShot::SetIntersectionEnable(bool enable) { intersectionEnable_ = enable; }

bool ObjShot::IsTempIntersectionMode() const
{
    return isFrameDeleteStarted_;
}

bool ObjShot::IsRegistered() const { return isRegistered_; }

void ObjShot::Regist()
{
	initX_ = GetX();
	initY_ = GetY();
	initSpeed_ = GetSpeed();
	initAngle_ = GetAngle();
	initDelay_ = GetDelay();
	initFadeDelete_ = fadeDeleteFrame_;
    isRegistered_ = true;
}

bool ObjShot::IsPlayerShot() const { return isPlayerShot_; }

const NullableSharedPtr<ShotData>& ObjShot::GetShotData() const
{
    return shotData_;
}

void ObjShot::SetShotData(const std::shared_ptr<ShotData>& data)
{
    if (data->texture)
    {
        shotData_ = data;
        ClearIntersection();
        if (shotData_)
        {
            angularVelocity_ = shotData_->angularVelocity;
            if (auto package = GetPackage().lock())
            {
                if (!isTempIntersectionMode_)
                {
                    for (const auto& col : shotData_->collisions)
                    {
                        AddIntersectionCircleA2(GetX() + col.x, GetY() + col.y, col.r);
                    }
                }
                if (shotData_->useAngularVelocityRand)
                {
                    angularVelocity_ = package->GetRandDouble(shotData_->angularVelocityRandMin, shotData_->angularVelocityRandMax);
                }
            }
        }
    }
}

int ObjShot::GetAnimationFrameCount() const
{
    return animationFrameCnt_;
}

int ObjShot::GetAnimationIndex() const
{
    return animationIdx_;
}

void ObjShot::AddShotA1(int shotObjId, int frame)
{
    if (IsDead()) return;
    if (auto package = GetPackage().lock())
    {
        if (auto shot = package->GetObject<ObjShot>(shotObjId))
        {
            shot->isRegistered_ = false;
            addedShots_.emplace_back(shotObjId, frame);
        }
    }
}

void ObjShot::AddShotA2(int shotObjId, int frame, float dist, float angle)
{
    if (IsDead()) return;
    if (auto package = GetPackage().lock())
    {
        if (auto shot = package->GetObject<ObjShot>(shotObjId))
        {
            shot->isRegistered_ = false;
            addedShots_.emplace_back(shotObjId, frame, dist, angle);
        }
    }
}

int ObjShot::GetFrameCountForAddShot() const
{
    return addedShotFrameCnt_;
}

const std::list<ObjShot::AddedShot>& ObjShot::GetAddedShot() const
{
    return addedShots_;
}

void ObjShot::GenerateBonusItem()
{
    if (auto package = GetPackage().lock())
    {
        package->GenerateBonusItem(GetX(), GetY());
    }
}

/*
void ObjShot::ToItem()
{
    if (IsDead()) return;
    if (IsItemChangeEnabled())
    {
        if (auto package = GetPackage().lock())
        {
            // EV_DELETE_SHOT_TO_ITEM 
            auto evArgs = std::make_unique<DnhArray>();
            evArgs->PushBack(std::make_unique<DnhReal>(GetID()));
            evArgs->PushBack(std::make_unique<DnhArray>(Point2D(GetX(), GetY())));
            if (auto itemScript = package->GetItemScript())
            {
                itemScript->NotifyEvent(EV_DELETE_SHOT_TO_ITEM, evArgs);
            }
            if (package->IsDeleteShotToItemEventOnShotScriptEnabled())
            {
                if (auto shotScript = package->GetShotScript())
                {
                    shotScript->NotifyEvent(EV_DELETE_SHOT_TO_ITEM, evArgs);
                }
            }
            GenerateBonusItem();
        }
    }
    Die();
}
*/

void ObjShot::ToItem()
{
	if (IsDead()) return;
	if (isFadeDeleteStarted_) return;
	if (IsItemChangeEnabled())
	{
		if (auto package = GetPackage().lock())
		{
			// EV_DELETE_SHOT_TO_ITEM 
			auto evArgs = std::make_unique<DnhArray>();
			evArgs->PushBack(std::make_unique<DnhReal>(GetID()));
			evArgs->PushBack(std::make_unique<DnhArray>(Point2D(GetX(), GetY())));
			if (package->IsDeleteShotToItemEventOnShotScriptEnabled())
			{
				package->NotifyEventAll(EV_DELETE_SHOT_TO_ITEM, evArgs);
			}
			GenerateBonusItem();
		}
	}
}

void ObjShot::EraseWithSpell()
{
	if (!IsSpellResistEnabled())
	{
		ToItem();
		FadeDelete();
	}
}

void ObjShot::DeleteImmediate()
{
	if (IsDead()) return;
	if (auto package = GetPackage().lock())
	{
		// EV_DELETE_SHOT_IMMEDIATE
		if (package->IsDeleteShotImmediateEventOnShotScriptEnabled())
		{
			//if(auto shotScript = package->GetShotScript())
			//{
			auto evArgs = std::make_unique<DnhArray>();
			evArgs->PushBack(std::make_unique<DnhReal>(GetID()));
			evArgs->PushBack(std::make_unique<DnhArray>(Point2D(GetX(), GetY())));
			package->NotifyEventAll(EV_DELETE_SHOT_IMMEDIATE, evArgs);
			//}
		}
	}
	Die();
}

void ObjShot::FadeDelete()
{
    if (!IsRegistered()) return;
    if (isFadeDeleteStarted_) return;
	fadeX_ = GetX();
	fadeY_ = GetY();
	fadeRandA_ = std::rand() % 360;
	fadeRandB_ = std::rand() % 360;
	fadeRandC_ = std::rand() % 20;
	fadeRandD_ = std::rand() % 16;
    isFadeDeleteStarted_ = true;
    fadeDeleteTimer_ = fadeDeleteFrame_;
}

float ObjShot::GetFadeScale() const
{
    if (IsFadeDeleteStarted())
    {
        return 1.0f * fadeDeleteTimer_ / fadeDeleteFrame_;
    }
    return 1.0f;
}

bool ObjShot::IsFadeDeleteStarted() const
{
    return isFadeDeleteStarted_;
}

bool ObjShot::IsFrameDeleteStarted() const
{
    return isFrameDeleteStarted_;
}

int ObjShot::GetDeleteFrameTimer() const
{
    return deleteFrameTimer_;
}

int ObjShot::GetFadeDeleteFrameTimer() const
{
    return fadeDeleteTimer_;
}

void ObjShot::SetDeleteFrame(int frame)
{
	isFrameDeleteStarted_ = true;
	deleteFrameTimer_ = frame;
}

void ObjShot::SetAutoDeleteDelay(int frame)  //FP AUTO TIMER
{
	autoDeleteTimer_ = frame;
}

void ObjShot::SetSpellResistDelay(int frame)  //FP SPELL RESIST DELAY
{
	spellResistTimer_ = frame;
}

void ObjShot::OnTrans(float dx, float dy)
{
    ObjCol::TransIntersection(dx, dy);
}

void ObjShot::RenderIntersection(const std::shared_ptr<Renderer>& renderer)
{
    ObjCol::RenderIntersection(renderer, IsPermitCamera(), GetPackage());
}

void ObjShot::CheckAutoDelete(float x, float y)
{
    if (auto package = GetPackage().lock())
    {
        if (autoDeleteEnable_ && package->IsOutOfShotAutoDeleteClip(x, y))
        {
            Die();
        }
    }
}

void ObjShot::UpdateAnimationPosition()
{
    if (shotData_)
    {
        if (animationIdx_ < 0 || animationIdx_ >= shotData_->animationData.size())
        {
            animationFrameCnt_ = animationIdx_ = 0;
            return;
        }
        animationFrameCnt_++;
        if (shotData_->animationData[animationIdx_].frame <= animationFrameCnt_)
        {
            animationFrameCnt_ = 0;
            animationIdx_++;
            if (animationIdx_ >= shotData_->animationData.size()) animationIdx_ = 0;
        }
    }
}

void ObjShot::TickDelayTimer()
{
    delayTimer_ = std::max(0, delayTimer_ - 1);
	if(IsDelay())
	{
		delayCounter_++;
	}
}

void ObjShot::TickDeleteFrameTimer()
{
	if (isFrameDeleteStarted_)
	{
		if (!IsDelay())
		{
			if (deleteFrameTimer_ <= 0)
			{
				FadeDelete();
			}
			deleteFrameTimer_--;
		}
	}
}

void ObjShot::TickAutoDeleteTimer() //FP AUTO TIMER
{
	if (autoDeleteTimer_ > -1)
	{
		autoDeleteEnable_ = false;
		if (autoDeleteTimer_ == 0)
		{
			autoDeleteEnable_ = true;
		}
		autoDeleteTimer_--;
	}
}

void ObjShot::TickSpellResistTimer() //FP SPELL RESIST DELAY
{
	if (spellResistTimer_ > -1)
	{
		spellResistEnable_ = true;
		if (spellResistTimer_ == 0)
		{
			spellResistEnable_ = false;
		}
		spellResistTimer_--;
	}
}

void ObjShot::TickAddedShotFrameCount()
{
    if (IsDelay() || IsDead()) return;
    auto it = addedShots_.begin();
    while (it != addedShots_.end())
    {
        if (it->frame == addedShotFrameCnt_)
        {
            if (auto package = GetPackage().lock())
            {
                if (auto shot = package->GetObject<ObjShot>(it->objId))
                {
                    float dx = 0;
                    float dy = 0;
                    if (it->type == AddedShot::Type::A1)
                    {
                        dx = shot->GetX();
                        dy = shot->GetX();
                    } else if (it->type == AddedShot::Type::A2)
                    {
                        float baseAngle = 0;
                        auto stLaser = dynamic_cast<ObjStLaser*>(this);
                        if (stLaser != NULL)
                        {
                            baseAngle = stLaser->GetLaserAngle();
                            shot->SetAngle(baseAngle + it->angle);
                        } else
                        {
                            baseAngle = GetAngle();
                        }
                        float dir = D3DXToRadian(baseAngle + it->angle);
                        dx = it->dist * cos(dir);
                        dy = it->dist * sin(dir);
                    }
                    shot->SetMovePosition(GetX() + dx, GetY() + dy);
                    shot->Regist();
                }
            }
            it = addedShots_.erase(it);
        } else
        {
            it++;
        }
    }
    addedShotFrameCnt_++;
}

void ObjShot::TickFadeDeleteTimer()
{
	if (!IsFadeDeleteStarted() || IsDead()) return;
	if (fadeDeleteTimer_ == 30)
	{
		if (auto package = GetPackage().lock())
		{
			//EV_DELETE_SHOT_FADE
			if (package->IsDeleteShotFadeEventOnShotScriptEnabled())
			{
				auto evArgs = std::make_unique<DnhArray>();
				evArgs->PushBack(std::make_unique<DnhReal>(GetID()));
				evArgs->PushBack(std::make_unique<DnhArray>(Point2D{ GetX(), GetY() }));
				package->NotifyEventAll(EV_DELETE_SHOT_FADE, evArgs);
			}
		}
	}
	if (fadeDeleteTimer_ <= 0)
	{
		Die();
	}
	fadeDeleteTimer_--;
	fadeDeleteCounter_++;
}

void ObjShot::Graze()
{
    isGrazeInvalid_ = true;
}

bool ObjShot::IsGrazeEnabled() const
{
    return !isGrazeInvalid_;
}

ObjShot::AddedShot::AddedShot(int objId, int frame) :
    type(Type::A1),
    objId(objId),
    frame(frame),
    dist(0),
    angle(0)
{
}

ObjShot::AddedShot::AddedShot(int objId, int frame, float dist, float angle) :
    type(Type::A2),
    objId(objId),
    frame(frame),
    dist(dist),
    angle(angle)
{
}

ObjLaser::ObjLaser(bool isPlayerShot, bool isECLShot, const std::shared_ptr<CollisionDetector>& colDetector, const std::shared_ptr<Package>& package) :
    ObjShot(isPlayerShot, isECLShot, colDetector, package),
    length_(0),
    renderWidth_(0),
    intersectionWidth_(0),
    hasIntersectionWidth_(false),
    grazeInvalidFrame_(20),
    grazeInvalidTimer_(0),
    itemDistance_(25)
{
    SetSpellResistEnable(true);
    SetPenetration(1 << 24);
}

void ObjLaser::SetShotData(const std::shared_ptr<ShotData>& shotData)
{
    this->shotData_ = shotData;
}

bool ObjLaser::IsGrazeEnabled() const
{
    return !isGrazeInvalid_ && grazeInvalidFrame_ > 0;
}

void ObjLaser::Graze()
{
    isGrazeInvalid_ = true;
    grazeInvalidTimer_ = grazeInvalidFrame_;
}

void ObjLaser::SetLength(float len)
{
    length_ = len;
}

float ObjLaser::GetRenderWidth() const { return renderWidth_; }

float ObjLaser::GetLength() const
{
    return length_;
}

void ObjLaser::SetRenderWidth(float width)
{
    renderWidth_ = width;
    if (!hasIntersectionWidth_)
    {
        SetIntersectionWidth(width / 2);
    }
}

float ObjLaser::GetIntersectionWidth() const
{
    return intersectionWidth_;
}

void ObjLaser::SetIntersectionWidth(float width)
{
    width = abs(width);
    if (intersectionWidth_ != width)
    {
        ObjCol::SetWidthIntersection(width);
        intersectionWidth_ = width;
    }
    hasIntersectionWidth_ = true;
}

float ObjLaser::GetGrazeInvalidFrame() const
{
    return grazeInvalidFrame_;
}

void ObjLaser::SetGrazeInvalidFrame(int frame)
{
    grazeInvalidFrame_ = frame;
}

float ObjLaser::GetGrazeInvalidTimer() const
{
    return grazeInvalidTimer_;
}

float ObjLaser::GetItemDistance() const
{
    return itemDistance_;
}

void ObjLaser::SetItemDistance(float distance)
{
    itemDistance_ = abs(distance);
}

void ObjLaser::TickGrazeInvalidTimer()
{
    if (isGrazeInvalid_)
    {
        grazeInvalidTimer_--;
    }
    if (grazeInvalidTimer_ == 0)
    {
        isGrazeInvalid_ = false;
    }
}

ObjLooseLaser::ObjLooseLaser(bool isPlayerShot, bool isECLShot, const std::shared_ptr<CollisionDetector>& colDetector, const std::shared_ptr<Package>& package) :
    ObjLaser(isPlayerShot, isECLShot, colDetector, package),
    renderLength_(0),
    invalidLengthHead_(0),
    invalidLengthTail_(0),
    defaultInvalidLengthEnable_(true)
{
    SetType(OBJ_LOOSE_LASER);
}

void ObjLooseLaser::Update()
{
    if (IsRegistered())
    {
        if (GetPenetration() <= 0) { Die(); return; }
        if (!IsDelay())
        {
            Move();
            Extend();
            Point2D tail = GetTail();
            CheckAutoDelete(tail.x, tail.y);
            UpdateAnimationPosition();
        }
        TickAddedShotFrameCount();
        TickDelayTimer();
        TickDeleteFrameTimer();
        TickFadeDeleteTimer();
        TickGrazeInvalidTimer();
    }
    UpdateTempIntersection();
}

void ObjLooseLaser::Render(const std::shared_ptr<Renderer>& renderer)
{
    if (IsRegistered())
    {
        if (const auto& shotData = GetShotData())
        {
            if (IsDelay())
            {
                ObjShot::Render(renderer);
            } else
            {
                RenderLaser(GetRenderWidth(), renderLength_, GetAngle(), renderer);
            }
        }
    }
}

void ObjLooseLaser::GenerateBonusItem()
{
	if (auto package = GetPackage().lock())
	{
		const Point2D head = GetHead();
		const Point2D tail = GetTail();
		const float dist = GetItemDistance();
		const float dx = dist * (tail.x - head.x) / GetRenderLength();
		const float dy = dist * (tail.y - head.y) / GetRenderLength();
		float distSum = 0;
		float x = head.x;
		float y = head.y;
		if (dist <= 0) return; // 無限ループ防止
		while (true)
		{
			package->GenerateBonusItem(x, y);
			if (package->IsDeleteShotToItemEventOnShotScriptEnabled())
			{
				auto evArgs = std::make_unique<DnhArray>();
				evArgs->PushBack(std::make_unique<DnhReal>(GetID()));
				evArgs->PushBack(std::make_unique<DnhArray>(Point2D(x, y)));
				package->NotifyEventAll(EV_DELETE_SHOT_TO_ITEM, evArgs);
			}
			x += dx;
			y += dy;
			distSum += dist;
			if (distSum > GetRenderLength()) break;
		}
	}
}

float ObjLooseLaser::GetInvalidLengthHead() const
{
    if (defaultInvalidLengthEnable_)
    {
        return GetRenderLength() * 0.1;
    }
    return invalidLengthHead_;
}

float ObjLooseLaser::GetInvalidLengthTail() const
{
    if (defaultInvalidLengthEnable_)
    {
        return GetRenderLength() * 0.1;
    }
    return invalidLengthTail_;
}

bool ObjLooseLaser::IsDefaultInvalidLengthEnabled() const
{
    return defaultInvalidLengthEnable_;
}

void ObjLooseLaser::SetDefaultInvalidLengthEnable(bool enable)
{
    defaultInvalidLengthEnable_ = enable;
}

void ObjLooseLaser::SetInvalidLength(float head, float tail)
{
    SetDefaultInvalidLengthEnable(false);
    invalidLengthHead_ = std::max(0.0f, head);
    invalidLengthTail_ = std::max(0.0f, tail);
}

Point2D ObjLooseLaser::GetHead() const
{
    return Point2D(GetX(), GetY());
}

Point2D ObjLooseLaser::GetTail() const
{
    float rdir = D3DXToRadian(GetAngle() + 180.0f); // reverse dir
    if (GetSpeed() < 0) rdir += D3DX_PI;
    float dx = GetRenderLength() * cos(rdir);
    float dy = GetRenderLength() * sin(rdir);
    return Point2D(GetX() + dx, GetY() + dy);
}

float ObjLooseLaser::GetRenderLength() const
{
    return renderLength_;
}

void ObjLooseLaser::UpdateIntersection()
{
    if (!isTempIntersectionMode_)
    {
        ClearIntersection();
        if (GetInvalidLengthHead() + GetInvalidLengthTail() <= GetRenderLength())
        {
            Point2D head = GetHead();
            Point2D tail = GetTail();
            // 新しい判定を追加
            // dir : head - tail
            float cosDir = (head.x - tail.x) / GetRenderLength();
            float sinDir = (head.y - tail.y) / GetRenderLength();
            float isectTailX = cosDir * GetInvalidLengthTail() + tail.x;
            float isectTailY = sinDir * GetInvalidLengthTail() + tail.y;
            float isectHeadX = -cosDir * GetInvalidLengthHead() + head.x;
            float isectHeadY = -sinDir * GetInvalidLengthHead() + head.y;
            AddIntersectionLine(isectTailX, isectTailY, isectHeadX, isectHeadY, GetIntersectionWidth());
        }
    }
}

void ObjLooseLaser::RenderLaser(float width, float length, float angle, const std::shared_ptr<Renderer>& renderer)
{
    if (const auto& shotData = GetShotData())
    {
        const auto head = GetHead();
        const auto tail = GetTail();
        float centerX = (head.x + tail.x) / 2;
        float centerY = (head.y + tail.y) / 2;

        /* ブレンド方法の選択 */
        // NOTE : shotDataのrenderは使わない
        int laserBlend = GetBlendType() == BLEND_NONE ? BLEND_ADD_ARGB : GetBlendType();
        int laserFilter = GetFilterType(); //FP FILTER

        // 色と透明度を設定
        D3DCOLOR color = GetColor().ToD3DCOLOR((int)(GetFadeScale() * std::min(shotData->alpha, GetAlpha())));
        auto vertices = GetRectVertices(color, shotData->texture->GetWidth(), shotData->texture->GetHeight(), (GetAnimationIndex() >= 0 && GetAnimationIndex() < shotData->animationData.size()) ? shotData->animationData[GetAnimationIndex()].rect : shotData->rect);

        /* 配置 */
        float rectWidth = abs(vertices[0].x - vertices[1].x);
        float rectHeight = abs(vertices[0].y - vertices[2].y);
        D3DXMATRIX world = CreateScaleRotTransMatrix(centerX, centerY, 0.0f, 0.0f, 0.0f, angle + 90.0f, width / rectWidth, length / rectHeight, 1.0f);

        renderer->RenderPrim2D(D3DPT_TRIANGLESTRIP, 4, vertices.data(), shotData->texture->GetTexture(), laserBlend, laserFilter, world, GetAppliedShader(), IsPermitCamera(), false);
    }
}

void ObjLooseLaser::Extend()
{
    float dl = abs(GetSpeed());
    if (renderLength_ + dl <= GetLength())
    {
        renderLength_ += dl;
        UpdateIntersection();
    }
}

ObjStLaser::ObjStLaser(bool isPlayerShot, bool isECLShot, const std::shared_ptr<CollisionDetector>& colDetector, const std::shared_ptr<Package>& package) :
    ObjLooseLaser(isPlayerShot, isECLShot, colDetector, package),
    laserAngle_(270),
    laserSourceEnable_(true),
    laserWidthScale_(0)
{
    SetType(OBJ_STRAIGHT_LASER);
}

void ObjStLaser::Update()
{
    if (IsRegistered())
    {
        if (GetPenetration() <= 0) Die();
        if (!IsDelay())
        {
            Move();
            CheckAutoDelete(GetX(), GetY());
            UpdateIntersection();
            UpdateAnimationPosition();
        }
        TickAddedShotFrameCount();
        TickDelayTimer();
        TickDeleteFrameTimer();
        TickFadeDeleteTimer();
        TickGrazeInvalidTimer();
        // 10フレームで1倍になるようにする
        laserWidthScale_ = IsDelay() ? 0.0f : std::min(1.0f, laserWidthScale_ + 0.1f);
    }
    UpdateTempIntersection();
}

void ObjStLaser::Render(const std::shared_ptr<Renderer>& renderer)
{
    if (IsRegistered())
    {
        if (const auto& shotData = GetShotData())
        {
            if (laserSourceEnable_ && !IsFadeDeleteStarted())
            {
                // レーザー源の描画
                auto vertices = GetRectVertices(shotData->delayColor.ToD3DCOLOR(0xff), shotData->texture->GetWidth(), shotData->texture->GetHeight(), shotData->delayRect);

                /* 配置 */
                const Point2D head = GetHead();
                float rectWidth = abs(vertices[0].x - vertices[1].x);
                float rectHeight = abs(vertices[0].y - vertices[2].y);
                float renderWidth = GetRenderWidth() * 1.3125f; // レーザーの幅よりちょっと大きい
                auto world = CreateScaleRotTransMatrix(head.x, head.y, 0.0f,
                                                       0.0f, 0.0f, GetLaserAngle() - 90.0f,
                                                       renderWidth / rectWidth, renderWidth / rectHeight, 1.0f);

                /* ブレンド方法の選択 */
                // NOTE :  delay_renderは使用しない
                int laserBlend = GetSourceBlendType() == BLEND_NONE ? BLEND_ADD_ARGB : GetSourceBlendType();
                int laserFilter = GetFilterType(); //FP FILTER
                renderer->RenderPrim2D(D3DPT_TRIANGLESTRIP, 4, vertices.data(), shotData->texture->GetTexture(), laserBlend, laserFilter, world, GetAppliedShader(), IsPermitCamera(), false);
            }
            // 遅延時間時は予告線
            float renderWidth = IsDelay() ? GetRenderWidth() / 20.0f : GetRenderWidth() * laserWidthScale_;
            renderWidth *= -1; // 左右反転してる
            RenderLaser(renderWidth, GetLength(), laserAngle_, renderer);
        }
    }
    RenderIntersection(renderer);
}

Point2D ObjStLaser::GetTail() const
{
    float dir = D3DXToRadian(laserAngle_);
    float tailX = GetX() + GetLength() * cos(dir);
    float tailY = GetY() + GetLength() * sin(dir);
    return Point2D(tailX, tailY);
}

float ObjStLaser::GetLaserAngle() const
{
    return laserAngle_;
}

void ObjStLaser::SetLaserAngle(float angle)
{
    laserAngle_ = angle;
}

bool ObjStLaser::HasSource() const
{
    return laserSourceEnable_;
}

void ObjStLaser::SetSource(bool source)
{
    laserSourceEnable_ = source;
}

float ObjStLaser::GetRenderLength() const
{
    return GetLength();
}

ObjCrLaser::ObjCrLaser(bool isPlayerShot, bool isECLShot, const std::shared_ptr<CollisionDetector>& colDetector, const std::shared_ptr<Package>& package) :
    ObjLaser(isPlayerShot, isECLShot, colDetector, package),
    totalLaserLength_(0),
    tailPos_(0),
    hasHead_(false),
    shrinkThresholdOffset_((size_t)(this) & 0xff),
    tipDecrement_(1)
{
    SetType(OBJ_CURVE_LASER);
}

void ObjCrLaser::Update()
{
    if (IsRegistered())
    {
        if (GetPenetration() <= 0) { Die(); return; }
        // 遅延時も動く
        Move();
        if (tailPos_ != trail_.size())
        {
            float tailX = (trail_[tailPos_].x + trail_[tailPos_ + 1].x) / 2;
            float tailY = (trail_[tailPos_].y + trail_[tailPos_ + 1].y) / 2;
            CheckAutoDelete(tailX, tailY);
        }
        Extend(GetX(), GetY());
        // NOTE : 描画にPrimitiveUpを使うので、trailはvectorである必要がある(メモリの連続性が必要)
        // 節の長さが上限を超えた時は末尾の節から削除していく必要があるが、毎フレームeraseを呼ぶのは遅いので
        // 末尾の位置(tailPos)を進めて、後でtailPosより前の部分をまとめてeraseする。
        // eraseするフレームが他のレーザーと被らないようにOffsetで閾値をずらしている
        // リプレイには影響ないはず
        if (tailPos_ >= (int)GetLength() + shrinkThresholdOffset_)
        {
            trail_.erase(trail_.begin(), trail_.begin() + tailPos_);
            tailPos_ = 0;
        }
        UpdateAnimationPosition();

        TickAddedShotFrameCount();
        TickDelayTimer();
        TickDeleteFrameTimer();
        TickFadeDeleteTimer();
        TickGrazeInvalidTimer();
    }
    UpdateTempIntersection();
}

void ObjCrLaser::Render(const std::shared_ptr<Renderer>& renderer)
{
    if (IsRegistered())
    {
        if (GetLaserNodeCount() <= 0) return;
        if (const auto& shotData = GetShotData())
        {
            if (IsDelay())
            {
                ObjShot::Render(renderer);
            } else
            {
                // uv算出用
                auto laserRect = GetRectVertices(0, shotData->texture->GetWidth(), shotData->texture->GetHeight(), (GetAnimationIndex() >= 0 && GetAnimationIndex() < shotData->animationData.size()) ? shotData->animationData[GetAnimationIndex()].rect : shotData->rect);

                // trailに色、UV値をセットする
                // レーザー中心の透明度
                const int baseAlpha = (int)(GetFadeScale() * std::min(GetAlpha(), shotData->alpha));
                // 先端~中心の減少の変化率, tipDecrementから0に変化するので負
                const float ddecr = -tipDecrement_ / (GetLaserNodeCount() >> 1);
                float lengthSum = 0;
                float decr = tipDecrement_;
                for (int i = tailPos_, lengthCnt = 0; i < trail_.size(); i += 2, lengthCnt++)
                {
                    Vertex& v1 = trail_[i];
                    Vertex& v2 = trail_[i + 1];
                    v1.u = laserRect[0].u;
                    v2.u = laserRect[1].u;
                    v1.v = v2.v = laserRect[2].v - lengthSum / totalLaserLength_ * (laserRect[2].v - laserRect[1].v);

                    int alpha = (int)(baseAlpha * (1 - abs(decr)));
                    decr += ddecr;

                    v1.color = v2.color = GetColor().ToD3DCOLOR(alpha);
                    if (lengthCnt < laserNodeLengthList_.size())
                    {
                        lengthSum += laserNodeLengthList_[lengthCnt];
                    }
                }

                // 描画
                D3DXMATRIX world;
                D3DXMatrixIdentity(&world);
                // ShotDataのrender値は使わない
                int laserBlend = GetBlendType() == BLEND_NONE ? BLEND_ADD_ARGB : GetBlendType();
                int laserFilter = GetFilterType(); //FP FILTER
                renderer->RenderPrim2D(D3DPT_TRIANGLESTRIP, trail_.size() - tailPos_, &trail_[tailPos_], shotData->texture->GetTexture(), laserBlend, laserFilter, world, GetAppliedShader(), IsPermitCamera(), false);
            }
        }
    }
    RenderIntersection(renderer);
}

void ObjCrLaser::GenerateBonusItem()
{
	int j = 4;
	if (auto package = GetPackage().lock())
	{
		if (GetLaserNodeCount() > 0)
		{
			for (int i = tailPos_; i < trail_.size(); i += 2)
			{
				float x = (trail_[i].x + trail_[i + 1].x) / 2;
				float y = (trail_[i].y + trail_[i + 1].y) / 2;

				if (j == 4)
				{
					auto evArgs = std::make_unique<DnhArray>();
					evArgs->PushBack(std::make_unique<DnhReal>(GetID()));
					evArgs->PushBack(std::make_unique<DnhArray>(Point2D(x, y)));
					if (package->IsDeleteShotToItemEventOnShotScriptEnabled())
					{
						package->NotifyEventAll(EV_DELETE_SHOT_TO_ITEM, evArgs);
					}
					j = -1;
				}
				j++;
			}
		}
	}
}

void ObjCrLaser::SetRenderWidth(float width)
{
    if (GetRenderWidth() != width)
    {
        FixVertexDistance_(width);
    }
    ObjLaser::SetRenderWidth(width);
}

float ObjCrLaser::GetTipDecrement() const
{
    return tipDecrement_;
}

void ObjCrLaser::FixVertexDistance_(float width)
{
    // 頂点の間隔を再設定
    float dw = width - GetRenderWidth();
    float prevX = 0;
    float prevY = 0;
    for (int i = tailPos_; i < trail_.size(); i += 2)
    {
        Vertex& v1 = trail_[i];
        Vertex& v2 = trail_[i + 1];

        float centerX = (v1.x + v2.x) / 2;
        float centerY = (v1.y + v2.y) / 2;

        if (i > tailPos_)
        {
            float rot = atan2(centerY - prevY, centerX - prevX) + D3DX_PI / 2;
            float dx = cos(rot) * dw / 2;
            float dy = sin(rot) * dw / 2;

            if (i == tailPos_ + 1)
            {
                trail_[i - 2].x += dx; trail_[i - 2].y += dy;
                trail_[i - 1].x -= dx; trail_[i - 1].y -= dy;
            }

            v1.x += dx; v1.y += dy;
            v2.x -= dx; v2.y -= dy;
        }

        prevX = centerX;
        prevY = centerY;
    }
}

void ObjCrLaser::SetTipDecrement(float dec)
{
    tipDecrement_ = constrain(dec, 0.0f, 1.0f);
}

void ObjCrLaser::Extend(float x, float y)
{
    if (hasHead_)
    {
        // 進行方向+90度
        const float normalDir = atan2(y - headY_, x - headX_) + D3DX_PI / 2.0f;
        const float halfWidth = GetRenderWidth() / 2.0f;
        const float dx = halfWidth * cos(normalDir);
        const float dy = halfWidth * sin(normalDir);

        if (tailPos_ == trail_.size())
        {
            trail_.emplace_back(headX_ + dx, headY_ + dy, 0.0f, 0, 0.0f, 0.0f);
            trail_.emplace_back(headX_ - dx, headY_ - dy, 0.0f, 0, 0.0f, 0.0f);
        }

        trail_.emplace_back(x + dx, y + dy, 0.0f, 0, 0.0f, 0.0f);
        trail_.emplace_back(x - dx, y - dy, 0.0f, 0, 0.0f, 0.0f);

        float laserNodeLength = std::hypotf(x - headX_, y - headY_);
        totalLaserLength_ += laserNodeLength;
        laserNodeLengthList_.push_back(laserNodeLength);

        AddIntersectionLine(headX_, headY_, x, y, GetIntersectionWidth());

        // カーブレーザーの場合
        // length : レーザーを構成する頂点の組の数
        if (GetLaserNodeCount() + 1 > (int)GetLength())
        {
            tailPos_ += 2;
            totalLaserLength_ -= laserNodeLengthList_[0];
            laserNodeLengthList_.pop_front();
            if (GetIntersections().size() > 0)
            {
                ObjCol::RemoveOldestIntersection();
            }
        }
    } else
    {
        hasHead_ = true;
    }
    headX_ = x;
    headY_ = y;
}

int ObjCrLaser::GetLaserNodeCount() const
{
    return laserNodeLengthList_.size();
}
}