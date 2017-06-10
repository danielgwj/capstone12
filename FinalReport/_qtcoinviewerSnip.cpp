
\lstset{backgroundcolor = \color{light-gray}, language=C++,
                basicstyle=\ttfamily,
                keywordstyle=\color{blue}\ttfamily,
                stringstyle=\color{red}\ttfamily,
                commentstyle=\color{green}\ttfamily,
                morecomment=[l][\color{magenta}]{\#}
}
\begin{lstlisting}
    /////////////
    //SHADER
    /////////////
    SoShapeHints * shapeHints = new SoShapeHints;
    shapeHints->vertexOrdering = SoShapeHints::CLOCKWISE;
    shapeHints->shapeType = SoShapeHints::SOLID;
    
    SoShapeHints * shapeHints1 = new SoShapeHints;
    shapeHints1->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;
    shapeHints1->shapeType = SoShapeHints::SOLID;
    
    SoSeparator* shaderSep = new SoSeparator;
    shaderSep->insertChild(QtCoinViewer::_ConfigureShaders(1), 0);
   
    SoSeparator* shaderSep1 = new SoSeparator;
    shaderSep1->insertChild(QtCoinViewer::_ConfigureShaders(0), 0);

    shaderSep->insertChild(shapeHints, 0);
    shaderSep->addChild(_ivBodies);
    shaderSep1->insertChild(shapeHints1, 0);
    shaderSep1->addChild(_ivBodies);

    SoDrawStyle *DrawStyle = new SoDrawStyle();
    DrawStyle->style = SoDrawStyle::LINES;
    
    DrawStyle->lineWidth = 4.;
    shaderSep->insertChild(DrawStyle,0);
    SoPolygonOffset* offset = new SoPolygonOffset;
    offset->styles.setValue(SoPolygonOffset::LINES);
    offset->factor = 6.0;
    shaderSep->insertChild(offset,0);
   
    /////////////
    //SHADOWS
    /////////////

    SoSpotLight* spotLight = new SoSpotLight;
    spotLight->intensity.setValue(0.1);
    spotLight->location.setValue(-0.015f,-0.02f,5);
    spotLight->color.setValue(1,1,1);
    spotLight->direction.setValue(0,0,-1); 
    spotLight->dropOffRate.setValue(0); 
    spotLight->cutOffAngle.setValue(0.78539819f);
    _ivBodies->addChild(spotLight);
 
    SoShadowStyle* shadowStyle = new SoShadowStyle;
    _ivBodies->addChild(shadowStyle);

    SoShadowGroup* shadowGroup = new SoShadowGroup;
    shadowGroup->addChild(_ivBodies);

    /////////////
    //END SHADER
    /////////////         
\end{lstlisting}
