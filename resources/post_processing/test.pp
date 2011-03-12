<posteffect name="blur">

  <pass copyBB="true" rt="BBCopy" enabled="true"/>

	<pass name="ping-pong" rt="BBCopyDiv2" clearRT="false" enabled="true">
    <material vertex="xyzuv" shader="ping_pong">
			<properties>
			  <property name="g_sourceTex" type="texture" value="BBCopy">
				  <sampler filter="BILINEAR" wrapping="CLAMP"/>
          <ui type="combobox"/>
				</property>
			</properties>
		</material>
	</pass>

  <pass name="ping-pong" rt="BBCopyDiv4" clearRT="false" enabled="true">
    <material vertex="xyzuv" shader="ping_pong">
      <properties>
        <property name="g_sourceTex" type="texture" value="BBCopyDiv2">
          <sampler filter="BILINEAR" wrapping="CLAMP"/>
          <ui type="combobox"/>
        </property>
      </properties>
    </material>
  </pass>

  <pass name="ping-pong" rt="BBCopyDiv2" clearRT="false" enabled="true">
    <material vertex="xyzuv" shader="ping_pong">
      <properties>
        <property name="g_sourceTex" type="texture" value="BBCopyDiv4">
          <sampler filter="BILINEAR" wrapping="CLAMP"/>
          <ui type="combobox"/>
        </property>
      </properties>
    </material>
  </pass>

  <pass name="ping-pong" rt="BBCopyDiv4" clearRT="false" enabled="true">
    <material vertex="xyzuv" shader="ping_pong">
      <properties>
        <property name="g_sourceTex" type="texture" value="BBCopyDiv2">
          <sampler filter="BILINEAR" wrapping="CLAMP"/>
          <ui type="combobox"/>
        </property>
      </properties>
    </material>
  </pass>

  <pass name="ping-pong" rt="BBCopyDiv2" clearRT="false" enabled="true">
    <material vertex="xyzuv" shader="ping_pong">
      <properties>
        <property name="g_sourceTex" type="texture" value="BBCopyDiv4">
          <sampler filter="BILINEAR" wrapping="CLAMP"/>
          <ui type="combobox"/>
        </property>
      </properties>
    </material>
  </pass>

  <pass name="ping-pong" rt="BBCopyDiv4" clearRT="false" enabled="true">
    <material vertex="xyzuv" shader="ping_pong">
      <properties>
        <property name="g_sourceTex" type="texture" value="BBCopyDiv2">
          <sampler filter="BILINEAR" wrapping="CLAMP"/>
          <ui type="combobox"/>
        </property>
      </properties>
    </material>
  </pass>

  <pass name="ping-pong" rt="BBCopyDiv2" clearRT="false" enabled="true">
    <material vertex="xyzuv" shader="ping_pong">
      <properties>
        <property name="g_sourceTex" type="texture" value="BBCopyDiv4">
          <sampler filter="BILINEAR" wrapping="CLAMP"/>
          <ui type="combobox"/>
        </property>
      </properties>
    </material>
  </pass>

  <pass name="ping-pong" rt="BB" clearRT="false" enabled="true">
    <material vertex="xyzuv" shader="ping_pong">
      <properties>
        <property name="g_sourceTex" type="texture" value="BBCopyDiv2">
          <sampler filter="BILINEAR" wrapping="CLAMP"/>
          <ui type="combobox"/>
        </property>
      </properties>
    </material>
  </pass>

</posteffect>