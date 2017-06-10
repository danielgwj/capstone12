
\lstset{backgroundcolor = \color{light-gray}, language=C++,
                basicstyle=\ttfamily,
                keywordstyle=\color{blue}\ttfamily,
                stringstyle=\color{red}\ttfamily,
                commentstyle=\color{green}\ttfamily,
                morecomment=[l][\color{magenta}]{\#}
}
\begin{lstlisting}
  gen.addDeclaration("varying vec3 diffuseColor;", FALSE);
  gen.addDeclaration("varying vec3 specularColor;", FALSE);
  gen.addDeclaration("varying vec3 ambientColor;", FALSE);
  gen.addDeclaration("varying float shininess;", FALSE);

  SbBool dirlight = FALSE;
  SbBool pointlight = FALSE;
  SbBool spotlight = FALSE;
  SbString str;

  gen.addMainStatement("vec4 ecPosition = gl_ModelViewMatrix * gl_Vertex;\n"
                       "ecPosition3 = ecPosition.xyz / ecPosition.w;");

  gen.addMainStatement("vec3 normal = normalize(gl_NormalMatrix * gl_Normal);\n"
                       "vec3 eye = -normalize(ecPosition3);\n"
                       "vec4 ambient;\n"
                       "vec4 diffuse;\n"
                       "vec4 specular;\n"
                       "vec4 accambient = vec4(0.0);\n"
                       "vec4 accdiffuse = vec4(0.0);\n"
                       "vec4 accspecular = vec4(0.0);\n"
                       "vec4 color;\n"
                       "diffuseColor = vec3(gl_FrontMaterial.diffuse);\n"
		       "specularColor = vec3(gl_FrontMaterial.specular);\n"
		       "ambientColor = vec3(gl_FrontMaterial.ambient);\n"
		       "shininess = gl_FrontMaterial.shininess;\n");

  gen.addMainStatement("fragmentNormal = normal;");

  if (!perpixelother) {
    for (i = 0; i < lights.getLength(); i++) {
      SoLight * l = (SoLight*) lights[i];
      if (l->isOfType(SoDirectionalLight::getClassTypeId())) {
        addDirectionalLight(gen, i);
        dirlight = TRUE;
      }
      else if (l->isOfType(SoSpotLight::getClassTypeId())) {
        addSpotLight(gen, i);
        spotlight = TRUE;
      }
      else if (l->isOfType(SoPointLight::getClassTypeId())) {
        addPointLight(gen, i);
        gen.addMainStatement(str);
        pointlight = TRUE;
      }
      else {
        SoDebugError::postWarning("SoShadowGroupP::setVertexShader",
                                  "Unknown light type: %s",
                                  l->getTypeId().getName().getString());
      }
      gen.addMainStatement("accambient += ambient; accdiffuse += diffuse; accspecular += specular;\n");
    }

    if (dirlight) gen.addNamedFunction(SbName("lights/DirectionalLight"), FALSE);
    if (pointlight) gen.addNamedFunction(SbName("lights/PointLight"), FALSE);

      //Changed all of this: made it look better
    /*gen.addMainStatement("color = gl_FrontLightModelProduct.sceneColor + "
                         "  accambient * gl_FrontMaterial.ambient + "
                         "  accdiffuse * gl_Color +"
                         "  accspecular * gl_FrontMaterial.specular;\n"*/
                         //);
  }
  else {
    gen.addMainStatement("color = gl_FrontLightModelProduct.sceneColor;\n");
  }

  if (numshadowlights) {
    gen.addMainStatement("vec4 pos = cameraTransform * ecPosition;\n"); // in world space
  }
  for (i = 0; i < numshadowlights; i++) {
    SoShadowLightCache * cache = this->shadowlights[i];
    SbString str;
    str.sprintf("shadowCoord%d = gl_TextureMatrix[%d] * pos;\n", i, cache->texunit); // in light space
    gen.addMainStatement(str);

    if (!perpixelspot) {
      spotlight = TRUE;
      addSpotLight(gen, cache->lightid);
      str.sprintf("spotVertexColor%d = \n"
                  "  ambient.rgb * gl_FrontMaterial.ambient.rgb + "
                  "  diffuse.rgb * gl_Color.rgb + "
                  "  specular.rgb * gl_FrontMaterial.specular.rgb;\n", i);
      gen.addMainStatement(str);
    }
  }

  if (spotlight) gen.addNamedFunction(SbName("lights/SpotLight"), FALSE);
  int32_t fogType = this->getFog(state);

  switch (fogType) {
  default:
    assert(0 && "unknown fog type");
  case SoEnvironmentElement::NONE:
    // do nothing
    break;
  case SoEnvironmentElement::HAZE:
  case SoEnvironmentElement::FOG:
  case SoEnvironmentElement::SMOKE:
    gen.addMainStatement("gl_FogFragCoord = abs(ecPosition3.z);\n");
    break;
  }
  gen.addMainStatement("perVertexColor = vec3(clamp(color.r, 0.0, 1.0), clamp(color.g, 0.0, 1.0), clamp(color.b, 0.0, 1.0));"
                       "gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n"
                       "gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord1;\n"
                       "gl_Position = ftransform();\n"
                       "gl_FrontColor = gl_Color;\n");

  if (this->hasclipplanes) {
    if (SoGLDriverDatabase::isSupported(glue, SO_GL_GLSL_CLIP_VERTEX_HW)) {
      gen.addMainStatement("gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;\n");
    }
  }

  // never update unless the program has actually changed. Creating a
  // new GLSL program is very slow on current drivers.
  if (this->vertexshader->sourceProgram.getValue() != gen.getShaderProgram()) {
    this->vertexshader->sourceProgram = gen.getShaderProgram();
    this->vertexshader->sourceType = SoShaderObject::GLSL_PROGRAM;
    this->vertexshadercache->set(gen.getShaderProgram());

    if (numshadowlights) {
      this->vertexshader->parameter.set1Value(0, this->cameratransform);
    }
    else {
      this->vertexshader->parameter.setNum(0);
    }
#if 0 // for debugging
    fprintf(stderr,"new vertex program: %s\n",
            gen.getShaderProgram().getString());
#endif
  }


  this->vertexshadercache->set(gen.getShaderProgram());

  state->pop();
  SoCacheElement::setInvalid(storedinvalid);

}

void
SoShadowGroupP::setFragmentShader(SoState * state)
{
  int i;

  SoShaderGenerator & gen = this->fragmentgenerator;
  gen.reset(FALSE);

  SbBool perpixelspot = FALSE;
  SbBool perpixelother = FALSE;
  this->getQuality(state, perpixelspot, perpixelother);

  const cc_glglue * glue = cc_glglue_instance(SoGLCacheContextElement::get(state));
  SbBool storedinvalid = SoCacheElement::setInvalid(FALSE);
  state->push();

  if (this->fragmentshadercache) {
    this->fragmentshadercache->unref();
  }
  this->fragmentshadercache = new SoShaderProgramCache(state);
  this->fragmentshadercache->ref();

  // set active cache to record cache dependencies
  SoCacheElement::set(state, this->fragmentshadercache);

  int numshadowlights = this->shadowlights.getLength();
  SbBool dirspot = FALSE;

  // ATi doesn't seem to support gl_FrontFace in hardware. We've only
  // verified that nVidia supports it so far.
  SbBool twosidetest = glue->vendor_is_nvidia && ((perpixelspot && numshadowlights) || perpixelother);


  if (numshadowlights) {
    SbString eps;
    eps.sprintf("const float EPSILON = %f;",
                PUBLIC(this)->epsilon.getValue());
    gen.addDeclaration(eps, FALSE);
    eps.sprintf("const float THRESHOLD = %f;",
                PUBLIC(this)->threshold.getValue());
    gen.addDeclaration(eps, FALSE);
  }
  for (i = 0; i < numshadowlights; i++) {
    SbString str;
    str.sprintf("uniform sampler2D shadowMap%d;", i);
    gen.addDeclaration(str, FALSE);

    str.sprintf("uniform float farval%d;", i);
    gen.addDeclaration(str, FALSE);

    str.sprintf("uniform float nearval%d;", i);
    gen.addDeclaration(str, FALSE);

    str.sprintf("varying vec4 shadowCoord%d;", i);
    gen.addDeclaration(str, FALSE);

    if (!perpixelspot) {
      str.sprintf("varying vec3 spotVertexColor%d;", i);
      gen.addDeclaration(str, FALSE);
    }
    if (this->shadowlights[i]->light->isOfType(SoDirectionalLight::getClassTypeId())) {
      str.sprintf("uniform vec4 lightplane%d;", i);
      gen.addDeclaration(str, FALSE);
    }
  }

  SbString str;
  if (numshadowlights) {
#ifdef DISTRIBUTE_FACTOR
    str.sprintf("const float DISTRIBUTE_FACTOR = %.1f;\n", DISTRIBUTE_FACTOR);
    gen.addDeclaration(str, FALSE);
#endif
  }
  gen.addDeclaration("varying vec3 ecPosition3;", FALSE);
  gen.addDeclaration("varying vec3 fragmentNormal;", FALSE);
  gen.addDeclaration("varying vec3 perVertexColor;", FALSE);
  //Added some declarations
  gen.addDeclaration("varying vec3 diffuseColor;", FALSE);
  gen.addDeclaration("varying vec3 specularColor;", FALSE);
  gen.addDeclaration("varying vec3 ambientColor;", FALSE);
  gen.addDeclaration("varying float shininess;", FALSE);

  const SoNodeList & lights = SoLightElement::getLights(state);

  if (numshadowlights) {
    gen.addNamedFunction("vsm/VsmLookup", FALSE);
  }
  gen.addMainStatement("vec3 normal = normalize(fragmentNormal);\n");
  if (twosidetest) {
    gen.addMainStatement("if (coin_two_sided_lighting != 0 && !gl_FrontFacing) normal = -normal;\n");
  }
  gen.addMainStatement("vec3 eye = -normalize(ecPosition3);\n");
  gen.addMainStatement("vec4 ambient = vec4(0.0);\n"
                       "vec4 diffuse = vec4(0.0);\n"
                       "vec4 specular = vec4(0.0);"
                       "vec4 mydiffuse = gl_Color;\n"
                       "vec4 texcolor = (coin_texunit0_model != 0) ? texture2D(textureMap0, gl_TexCoord[0].xy) : vec4(1.0);\n");

  if (this->numtexunitsinscene > 1) {
    gen.addMainStatement("if (coin_texunit1_model != 0) texcolor *= texture2D(textureMap1, gl_TexCoord[1].xy);\n");
  }

gen.setVersion("#version 130");

	//Warm cool shading happens here

gen.addMainStatement(  //"vec3 color = perVertexColor;\n"
	     	       "vec3 color;\n"
		       "const vec3 lightColor = vec3(1.0);\n"
                       "const vec3 specColor  = vec3(1.0);\n"
		       "const vec3 globalAmbient = vec3(0.2);\n"
                       "vec3 position_eyespace = ecPosition3;\n"
		       "vec3 normal_eyespace = normal;\n"
                       "vec3 myAmb = ambientColor * globalAmbient;\n"
		       "vec3 light_direction = normalize(vec3(gl_LightSource[0].position) - position_eyespace);\n"
		       "float NL = max(dot(normal_eyespace, light_direction), 0);\n"
		       "vec3 myDiff = vec3(1.,1.,1.) * lightColor * NL;\n"
                       "float coolScalar = 0.25;\n"
		       "float warmScalar = 0.25;\n"
		       "vec3 Kcool;\n"
		       "vec3 Kwarm;\n"
		       "vec3 coolcolor = vec3(0.,0.,0.);\n"
		       "vec3 warmcolor = vec3(0.,0.,0.);\n"
		       "vec3 V = normalize(-position_eyespace);\n"
		       "vec3 H = normalize(light_direction + V);\n"
                       
                       "if(diffuseColor.x >= diffuseColor.y + 0.15 && diffuseColor.x >= diffuseColor.z+0.15){\n"
		//Warmcool for Red
		       "coolcolor = vec3(1.,0.,0.);\n"
		       "warmcolor = vec3(1.,1.,0.); }\n"

		       "else if(diffuseColor.y >= diffuseColor.x + 0.15 && diffuseColor.y >= diffuseColor.x + 0.15){\n"
		//Warmcool for Green
		       "coolcolor = vec3(0.,0.,1.);\n"
		       "warmcolor = vec3(0.,1.,0.); }\n"

	               "else if(diffuseColor.z >= diffuseColor.x + 0.15 && diffuseColor.z >= diffuseColor.y + 0.15){\n"
		//Warmcool for Blue
	 	       "coolcolor = vec3(1.,0.,1.);\n"
		       "warmcolor = vec3(0.,0.,1.); }\n"

		       "else if(diffuseColor.x > diffuseColor.z+0.15 && diffuseColor.y > diffuseColor.z+0.15 && abs(diffuseColor.x - diffuseColor.y) < 0.15){\n"
		//Warmcool for yellow
		       "coolcolor = vec3(1.,1.,1.);\n"
		       "warmcolor = vec3(0.,1.,1.); }\n"

	 	       "else if(diffuseColor.x > diffuseColor.y+0.15 && diffuseColor.z > diffuseColor.y+0.15 && abs(diffuseColor.x - diffuseColor.z) < 0.15){\n"
		//Warmcool for purple
		       "coolcolor = vec3(0.,1.,1.);\n"
		       "warmcolor = vec3(0.,0.,1.); }\n"

		       "else if(diffuseColor.z > diffuseColor.x+0.15 && diffuseColor.y > diffuseColor.x+0.15 && abs(diffuseColor.z - diffuseColor.y) < 0.15){\n"
		//Warmcool for cyan
		       "coolcolor = vec3(0.,0.,1.);\n"
		       "warmcolor = vec3(0.,1.,1.); }\n"

		       "else if(diffuseColor.x > 0.5 && diffuseColor.y > 0.5 && diffuseColor.z > 0.5){\n"
		//Warmcool for white
		       "coolcolor = vec3(0.,0.,1.);\n"
		       "warmcolor = vec3(1.,1.,1.); }\n"

		       "else if(diffuseColor.x <= 0.5 && diffuseColor.y <=0.5 && diffuseColor.z <= 0.5){\n"
		//Warmcool for black
		       "coolcolor = vec3(0., 0., 1.);\n"
		       "warmcolor = vec3(0.8,0.8,0.8); }\n"	

		       "Kcool = coolcolor + coolScalar * diffuseColor.xyz;\n"
		       "Kwarm = warmcolor + warmScalar * diffuseColor.xyz;\n"

		       "float newShine = shininess;\n"

		       "newShine = 10.0f * newShine;\n"

		       "float specularLight = pow(max(dot(normal_eyespace, H),0), newShine);\n"
		       "vec3 mySpec = specColor * lightColor * specularLight; \n"
		       "color = mix(Kcool,Kwarm, myDiff) + mySpec + myAmb;\n"
                       "vec3 scolor = vec3(0.0);\n"
                       "float dist;\n"
                       "float shadeFactor;\n"
                       "vec3 coord;\n"
                       "vec4 map;\n"
                       "mydiffuse.a *= texcolor.a;\n");

  if (perpixelspot) {
    SbBool spotlight = FALSE;
    SbBool dirlight = FALSE;
    for (i = 0; i < numshadowlights; i++) {
      SoShadowLightCache * cache = this->shadowlights[i];
      SbBool dirshadow = FALSE;
      SbString str;
      SbBool normalspot = FALSE;
      SbString insidetest = "&& coord.x >= 0.0 && coord.x <= 1.0 && coord.y >= 0.0 && coord.y <= 1.0)";

      SoLight * light = this->shadowlights[i]->light;
      if (light->isOfType(SoSpotLight::getClassTypeId())) {
        SoSpotLight * sl = static_cast<SoSpotLight*> (light);
        if (sl->dropOffRate.getValue() >= 0.0f) {
          insidetest = ")";
          spotlight = TRUE;
          normalspot = TRUE;
        }
        else {
          insidetest = ")";
          dirspot = TRUE;
        }
      }
      else {
        dirshadow = TRUE;
        dirlight = TRUE;
      }
      if (dirshadow) {
        str.sprintf("dist = dot(ecPosition3.xyz, lightplane%d.xyz) - lightplane%d.w;\n", i,i);
        gen.addMainStatement(str);
        addDirectionalLight(gen, cache->lightid);
      }
      else {
        if (normalspot) {
          addSpotLight(gen, cache->lightid, TRUE);
        }
        else {
          addDirSpotLight(gen, cache->lightid, TRUE);
        }
      }
      str.sprintf("coord = 0.5 * (shadowCoord%d.xyz / shadowCoord%d.w + vec3(1.0));\n", i , i);
      gen.addMainStatement(str);
      str.sprintf("map = texture2D(shadowMap%d, coord.xy);\n", i);
      gen.addMainStatement(str);
#ifdef USE_NEGATIVE
      gen.addMainStatement("map = (map + vec4(1.0)) * 0.50;\n");
#endif // USE_NEGATIVE
#ifdef DISTRIBUTE_FACTOR
      gen.addMainStatement("map.xy += map.zw / DISTRIBUTE_FACTOR;\n");
#endif
      str.sprintf("shadeFactor = ((map.x < 0.9999) && (shadowCoord%d.z > -1.0 %s) "
                  "? VsmLookup(map, (dist - nearval%d) / (farval%d - nearval%d), EPSILON, THRESHOLD) : 1.0;\n",
                  i, insidetest.getString(),i,i,i);
      gen.addMainStatement(str);

      if (dirshadow) {
        SoShadowDirectionalLight * sl = static_cast<SoShadowDirectionalLight*> (light);
        if (sl->maxShadowDistance.getValue() > 0.0f) {
          gen.addMainStatement("shadeFactor = 1.0 - shadeFactor;\n");

          // linear falloff
          // str.sprintf("shadeFactor *= max(0.0, min(1.0, 1.0 + ecPosition3.z/maxshadowdistance%d));\n", i);

          // See SoGLEnvironemntElement.cpp (updategl()) to see how the magic exp() constants here are calculated

          // exp(f) falloff
          // str.sprintf("shadeFactor *= min(1.0, exp(5.545*ecPosition3.z/maxshadowdistance%d));\n", i);
          // just use exp(f^2) as a falloff formula for now, consider making this configurable
          str.sprintf("shadeFactor *= min(1.0, exp(2.35*ecPosition3.z*abs(ecPosition3.z)/(maxshadowdistance%d*maxshadowdistance%d)));\n", i,i);
          gen.addMainStatement(str);
          gen.addMainStatement("shadeFactor = 1.0 - shadeFactor;\n");
        }
      }
      //Changed this around
      //Could be optimized
      //gen.addMainStatement("shadeFactor = pow(shadeFactor, 50);\n");
      gen.addMainStatement("float tempShade = shadeFactor;\n"
			   "tempShade = pow(tempShade, 50);\n"
			   "if(tempShade >= 0.90f){\n"
			   "color += shadeFactor * diffuse.rgb * myDiff.rgb;}\n"
                           "else {\n" 
                           "color -= vec3(0.3f,0.3f,0.3f); }\n");
      //gen.addMainStatement("color += shadeFactor * diffuse.rgb * mydiffuse.rgb;");
      gen.addMainStatement("scolor += shadeFactor * gl_FrontMaterial.specular.rgb * specular.rgb;\n");
\end{lstlisting}
