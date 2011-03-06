<posteffect name="ssaa">
	
	<!--
	<pass name="copy" rt="BBCopy" clearRT="false" enabled="false">
		<material name="copyBB">
		  <properties/>
		</material>
		<renderStates/>
	</pass>
	-->
	
	<pass name="ssaa" rt="BB" clearRT="false" enabled="true">
		<material vertex="xyzuv" shader="ssaa">
		  <properties>
			<property name="g_filterWidth" type="float" value="1.75">
				<ui type="slider" range="vec2f(0,5)" step="0.01"/>
			</property>
			<property name="g_threshold" type="float" value="0.0625">
				<ui type="slider" range="vec2f(0,1)" step="0.001"/>
			</property>
			<property name="g_debug" type="float" value="0">
				<ui type="checkbox"/>
			</property>
			<property name="g_sourceTex" type="texture" value="BBCopy"/>
		  </properties>
		</material>
		<renderStates>
			<!-- <state name="depthEnabled" value="true"/> -->
		</renderStates>
	</pass>
	
	<!-- ... another posteffect's passes put here. -->	
		
</posteffect>